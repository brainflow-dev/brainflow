"""Hardware-free RDA session test. Pass the native BoardController library path."""
import ctypes
import json
import socket
import struct
import sys
import threading
import time

UUID = bytes.fromhex('8e45584396c9864caf4a98bbf6c91450')


def message(kind, body):
    return UUID + struct.pack('<II', len(body) + 24, kind) + body


def main(library):
    lib = ctypes.CDLL(library)
    lib.prepare_session.argtypes = [ctypes.c_int, ctypes.c_char_p]
    lib.start_stream.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p]
    lib.stop_stream.argtypes = lib.prepare_session.argtypes
    lib.release_session.argtypes = lib.prepare_session.argtypes
    lib.get_current_board_data.argtypes = [ctypes.c_int, ctypes.c_int,
        ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_char_p]
    lib.config_board.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.POINTER(ctypes.c_int),
        ctypes.c_int, ctypes.c_int, ctypes.c_char_p]
    server = socket.socket()
    server.bind(('127.0.0.1', 0))
    server.listen()
    server.settimeout(0.1)
    stop = threading.Event()
    mode = ['valid']
    workers = []
    failures = []
    params = json.dumps(dict(serial_port='', mac_address='', ip_address='127.0.0.1',
        ip_address_aux='', ip_address_anc='', ip_port=server.getsockname()[1], ip_port_aux=0,
        ip_port_anc=0, ip_protocol=0, other_info='', timeout=1, serial_number='',
        file='', file_aux='', file_anc='', master_board=-100)).encode()

    def peer(conn, scenario):
        try:
            with conn:
                if scenario == 'silent':
                    stop.wait(3)
                    return
                if scenario == 'oversized':
                    conn.sendall(UUID + struct.pack('<II', 0xffffffff, 4))
                    return
                channels = 31 if scenario == 'profile' else 32
                start = struct.pack('<Id', channels, 1000) + struct.pack('<32d', *([0.5] * 32))
                start += b''.join(('C%d\0' % i).encode() for i in range(32))
                packet = message(1, start)
                for offset in range(0, len(packet), 7):
                    conn.sendall(packet[offset:offset + 7])
                if scenario == 'profile':
                    return
                block = 0
                while not stop.is_set():
                    label = b'S\xff12' if scenario == 'encoding' else b'S 12'
                    marker = struct.pack('<IIII', 30, 1, 1, 0xffffffff) + b'Stimulus\0' + label + b'\0'
                    body = struct.pack('<III', block, 2, 2) + struct.pack('<64f', *([-3.5] * 64))
                    packet = message(4, body + marker * 2)
                    for offset in range(0, len(packet), 17):
                        conn.sendall(packet[offset:offset + 17])
                    if scenario == 'stop':
                        conn.sendall(message(3, b''))
                        return
                    block += 1
                    stop.wait(0.004)
        except (BrokenPipeError, ConnectionResetError):
            pass  # prepare/start and stop deliberately close earlier connections
        except Exception as exc:
            failures.append(exc)

    def accept():
        while not stop.is_set():
            try:
                conn, _ = server.accept()
            except socket.timeout:
                continue
            worker = threading.Thread(target=peer, args=(conn, mode[0]))
            workers.append(worker)
            worker.start()

    listener = threading.Thread(target=accept)
    listener.start()
    try:
        assert lib.prepare_session(71, params) == 0
        for _ in range(2):
            assert lib.start_stream(1024, b'', 71, params) == 0
            time.sleep(0.05)
            data = (ctypes.c_double * (37 * 1024))()
            count = ctypes.c_int()
            assert lib.get_current_board_data(1024, 0, data, ctypes.byref(count), 71, params) == 0
            assert count.value >= 2
            assert all(data[count.value + i] == -1.75 for i in range(count.value))
            assert 2 in data[33 * count.value:34 * count.value]
            response = ctypes.create_string_buffer(8192)
            length = ctypes.c_int()
            assert lib.config_board(b'get_marker', response, ctypes.byref(length), 8192, 71, params) == 0
            event = json.loads(response.value)
            assert event['description'] == 'S 12' and event['position'] == 1
            assert lib.config_board(b'get_channel_names', response, ctypes.byref(length), 8192, 71, params) == 0
            assert json.loads(response.value) == ['C%d' % i for i in range(32)]
            assert lib.stop_stream(71, params) == 0
        for scenario, expected in [('encoding', 0), ('profile', 5), ('oversized', 5), ('silent', 18), ('stop', 0)]:
            mode[0] = scenario
            before = time.monotonic()
            assert lib.start_stream(1024, b'', 71, params) == expected, scenario
            assert time.monotonic() - before < 2.5, 'stop did not interrupt recv'
            if scenario == 'encoding':
                assert lib.config_board(b'get_marker', response, ctypes.byref(length), 8192, 71, params) == 0
                assert json.loads(response.value)['description'] == 'S\ufffd12'
            if expected == 0:
                assert lib.stop_stream(71, params) == 0
        assert lib.release_session(71, params) == 0
        mode[0] = 'valid'
        assert lib.prepare_session(71, params) == 0
        assert lib.start_stream(1024, b'', 71, params) == 0
    finally:
        lib.release_session(71, params)
        stop.set()
        listener.join()
        for worker in workers:
            worker.join()
        server.close()
    assert not failures, failures
    print('RDA protocol and session lifecycle passed')


if __name__ == '__main__':
    main(sys.argv[1])
