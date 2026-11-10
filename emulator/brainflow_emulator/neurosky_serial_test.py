"""Exercise the native driver with a POSIX pseudo-terminal, no device required.

Usage: python3 neurosky_serial_test.py /path/to/libBoardController.so
"""
import ctypes
import json
import os
import pty
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

    def device():
        # A raw value of 42; the checksum is independently specified.
        packet = bytes([0xaa, 0xaa, 4, 0x80, 2, 0, 42, 0x53])
        while not stop.wait(0.002):
            if active.is_set():
                os.write(master, packet[:3])
                os.write(master, packet[3:])

    worker = threading.Thread(target=device)
    worker.start()
    try:
        assert lib.prepare_session(69, params) == 0
        # No raw samples: start must fail, and its worker must be joined.
        assert lib.start_stream(1024, b'', 69, params) == 18
        active.set()
        for _ in range(2):
            assert lib.start_stream(1024, b'', 69, params) == 0
            assert lib.start_stream(1024, b'', 69, params) == 8
            time.sleep(0.05)
            data = (ctypes.c_double * (15 * 1024))()
            count = ctypes.c_int()
            assert lib.get_current_board_data(1024, 0, data, ctypes.byref(count), 69, params) == 0
            assert count.value > 1
            assert all(data[count.value + i] == 42 * 0.2197265625 for i in range(count.value))
            assert lib.stop_stream(69, params) == 0
            assert lib.stop_stream(69, params) == 11
        assert lib.release_session(69, params) == 0
        # Opening the port again after release must work.
        assert lib.prepare_session(69, params) == 0
        assert lib.start_stream(1024, b'', 69, params) == 0
    finally:
        lib.release_session(69, params)
        stop.set()
        worker.join()
        os.close(master)
        os.close(slave)
    print('NeuroSky serial lifecycle passed')


if __name__ == '__main__':
    main(sys.argv[1])
