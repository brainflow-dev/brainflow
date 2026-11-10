"""NA400 command/data loopback test; pass the native BoardController library path."""
import ctypes
import json
import socket
import struct
import sys
import threading
import time


def main(library):
    lib = ctypes.CDLL(library)
    lib.prepare_session.argtypes = [ctypes.c_int, ctypes.c_char_p]
    lib.start_stream.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p]
    lib.stop_stream.argtypes = lib.prepare_session.argtypes
    lib.release_session.argtypes = lib.prepare_session.argtypes
    lib.get_current_board_data.argtypes = [ctypes.c_int, ctypes.c_int,
        ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_char_p]
    servers = []
    for _ in range(2):
        server = socket.socket()
        server.bind(('127.0.0.1', 0))
        server.listen()
        server.settimeout(0.1)
        servers.append(server)
    stop = threading.Event()
    streaming = threading.Event()
    mode = ['valid']
    workers = []
    failures = []
    commands = []
    params = json.dumps(dict(serial_port='', mac_address='', ip_address='127.0.0.1',
        ip_address_aux='', ip_address_anc='', ip_port=servers[0].getsockname()[1],
        ip_port_aux=servers[1].getsockname()[1], ip_port_anc=0, ip_protocol=0,
        other_info='', timeout=1, serial_number='', file='', file_aux='', file_anc='',
        master_board=-100)).encode()

    def command_peer(conn):
        with conn:
            pending = b''
            conn.settimeout(0.1)
            while not stop.is_set():
                try:
                    chunk = conn.recv(1024)
                except socket.timeout:
                    continue
                if not chunk:
                    return
                pending += chunk
                while b'\n' in pending:
                    line, pending = pending.split(b'\n', 1)
                    parts = line.decode().strip('()').split()
                    assert len(parts) == 5 and parts[0] == 'sendCommand' and parts[2:4] == ['0', '0']
                    name = parts[1]
                    commands.append((name, parts[4]))
                    response = '(sendCommand_return (status complete))\n'
                    if name == 'cmd_GetAmpDetails':
                        model = 'NA500' if mode[0] == 'model' else 'NA400'
                        response = ('(sendCommand_return (status complete) (amp_details '
                            '(amp_type %s) (packet_format 2) (number_of_channels 256)))\n') % model
                    elif name == 'cmd_Stop':
                        streaming.clear()
                    elif name == 'cmd_Start':
                        streaming.set()
                    elif name == 'cmd_SetDecimatedRate':
                        assert parts[4] == '1000'
                    else:
                        assert name in ('cmd_SetPower', 'cmd_DefaultAcquisitionState')
                    for i in range(0, len(response), 5):
                        conn.sendall(response[i:i + 5].encode())

    def data_peer(conn):
        with conn:
            conn.settimeout(0.1)
            pending = b''
            while b'\n' not in pending and not stop.is_set():
                try:
                    chunk = conn.recv(1024)
                except socket.timeout:
                    continue
                if not chunk:
                    return
                pending += chunk
            assert pending.strip() == b'(sendCommand cmd_ListenToAmp 0 0 0)'
            seq = 0
            scenario = mode[0]
            while not stop.wait(0.005):
                if not streaming.is_set() or scenario == 'silent':
                    continue
                frames = []
                for counter in [seq, seq, seq + 1]:  # duplicate must be discarded
                    frame = bytearray(1264)
                    struct.pack_into('<H', frame, 0, 0xfffe)
                    struct.pack_into('<Q', frame, 25, counter)
                    struct.pack_into('<Q', frame, 33, 123000 + counter * 1000)
                    frame[41] = 5 if scenario == 'net' else 6
                    struct.pack_into('<256i', frame, 80, *([-1000000] * 256))
                    frames.append(frame)
                body = b''.join(frames)
                packet = struct.pack('>QQ', 2 if scenario == 'id' else 0, len(body)) + body
                for i in range(0, len(packet), 113):
                    conn.sendall(packet[i:i + 113])
                seq += 2

    def guarded(conn, handler):
        try:
            handler(conn)
        except (BrokenPipeError, ConnectionResetError):
            pass
        except Exception as exc:
            failures.append(exc)

    def accept(server, handler):
        while not stop.is_set():
            try:
                conn, _ = server.accept()
            except socket.timeout:
                continue
            worker = threading.Thread(target=guarded, args=(conn, handler))
            workers.append(worker)
            worker.start()

    listeners = [threading.Thread(target=accept, args=(server, handler))
        for server, handler in zip(servers, [command_peer, data_peer])]
    for listener in listeners:
        listener.start()
    try:
        mode[0] = 'model'
        assert lib.prepare_session(72, params) == 6
        mode[0] = 'valid'
        assert lib.prepare_session(72, params) == 0
        for _ in range(2):
            assert lib.start_stream(1024, b'', 72, params) == 0
            time.sleep(0.05)
            data = (ctypes.c_double * (264 * 1024))()
            count = ctypes.c_int()
            assert lib.get_current_board_data(1024, 0, data, ctypes.byref(count), 72, params) == 0
            assert count.value >= 2
            assert all(data[count.value + i] == -1000000 * 0.00015522042 for i in range(count.value))
            assert all(data[i + 1] - data[i] == 1 for i in range(count.value - 1))
            assert all(data[258 * count.value + i] == 1 for i in range(count.value))
            assert lib.stop_stream(72, params) == 0
        for scenario, expected in [('id', 5), ('net', 5), ('silent', 18)]:
            mode[0] = scenario
            assert lib.start_stream(1024, b'', 72, params) == expected, scenario
        mode[0] = 'valid'
        assert lib.start_stream(1024, b'', 72, params) == 0
        assert lib.release_session(72, params) == 0
        assert lib.prepare_session(72, params) == 0
        assert lib.start_stream(1024, b'', 72, params) == 0
    finally:
        lib.release_session(72, params)
        stop.set()
        for listener in listeners:
            listener.join()
        for worker in workers:
            worker.join()
        for server in servers:
            server.close()
    assert not failures, failures
    assert ('cmd_SetDecimatedRate', '1000') in commands
    assert commands[-1] == ('cmd_Stop', '0')
    print('EGI NA400 command/data lifecycle passed')


if __name__ == '__main__':
    main(sys.argv[1])
