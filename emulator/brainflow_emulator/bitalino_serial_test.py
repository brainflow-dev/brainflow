"""Exercise the native driver with a POSIX pseudo-terminal, no device required.

Usage: python3 bitalino_serial_test.py /path/to/libBoardController.so
"""
import ctypes
import json
import os
import pty
import select
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
        ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_int),
        ctypes.c_int, ctypes.c_char_p]
    master, slave = pty.openpty()
    params = json.dumps(dict(serial_port=os.ttyname(slave), mac_address='',
        ip_address='', ip_address_aux='', ip_address_anc='', ip_port=0,
        ip_port_aux=0, ip_port_anc=0, ip_protocol=0, other_info='', timeout=1,
        serial_number='', file='', file_aux='', file_anc='', master_board=-100)).encode()
    stop = threading.Event()
    active = threading.Event()

    commands = []
    failures = []

    def device():
        table = [0, 3, 6, 5, 12, 15, 10, 9, 11, 8, 13, 14, 7, 4, 1, 2]
        streaming = False
        seq = 0
        try:
            while not stop.is_set():
                readable, _, _ = select.select([master], [], [], 0.002)
                if readable:
                    for command in os.read(master, 64):
                        commands.append(command)
                        if command == 0:
                            streaming = False
                        elif command == 7:
                            os.write(master, b'BITalino_v5.0\n')
                        elif command in (0xfd, 0xfe):
                            assert len(commands) >= 2 and commands[-2] == 0xc3
                            streaming = True
                            seq = 0
                        else:
                            assert command == 0xc3
                if streaming and active.is_set():
                    packet = [209, 175, 106, 85, 0, 254, 175, seq << 4]
                    crc = 0
                    for value in packet[:-1]:
                        crc = table[crc] ^ (value >> 4)
                        crc = table[crc] ^ (value & 15)
                    packet[-1] |= table[table[crc] ^ seq]
                    os.write(master, bytes(packet[:3]))
                    os.write(master, bytes(packet[3:]))
                    seq = (seq + 1) % 16
        except Exception as exc:
            failures.append(exc)

    worker = threading.Thread(target=device)
    worker.start()
    try:
        assert lib.prepare_session(70, params) == 0
        # No raw samples: start must fail, and its worker must be joined.
        assert lib.start_stream(1024, b'', 70, params) == 18
        active.set()
        for _ in range(2):
            assert lib.start_stream(1024, b'', 70, params) == 0
            assert lib.start_stream(1024, b'', 70, params) == 8
            time.sleep(0.05)
            data = (ctypes.c_double * (13 * 1024))()
            count = ctypes.c_int()
            assert lib.get_current_board_data(1024, 0, data, ctypes.byref(count), 70, params) == 0
            assert count.value > 1
            expected = [1023, 512, 341, 682, 63, 17, 1, 0, 1, 0]
            for channel, value in enumerate(expected, 1):
                assert all(data[channel * count.value + i] == value for i in range(count.value))
            assert lib.stop_stream(70, params) == 0
            assert lib.stop_stream(70, params) == 11
        assert lib.release_session(70, params) == 0
        # Opening the port again after release must work.
        assert lib.prepare_session(70, params) == 0
        assert lib.start_stream(1024, b'', 70, params) == 0
    finally:
        lib.release_session(70, params)
        stop.set()
        worker.join()
        os.close(master)
        os.close(slave)
    assert not failures, failures
    assert commands[:2] == [0, 7]
    print('BITalino serial lifecycle passed')


if __name__ == '__main__':
    main(sys.argv[1])
