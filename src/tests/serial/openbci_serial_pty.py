"""POSIX integration regression against a freshly built BoardController (no third-party packages)."""

import ctypes
import json
import os
import pty
import select
import sys
import threading
import time


def params(**overrides):
    result = {key: "" for key in (
        "serial_port", "serial_number", "mac_address", "other_info", "ip_address",
        "ip_address_aux", "ip_address_anc", "file", "file_aux", "file_anc")}
    result.update(ip_port=0, ip_port_aux=0, ip_port_anc=0, ip_protocol=0, timeout=0, master_board=-100)
    result.update(overrides)
    return json.dumps(result).encode()


class Dongle:
    def __init__(self, delay=0, silent=False):
        self.master, self.slave = pty.openpty()
        self.name = os.ttyname(self.slave)
        self.delay = delay
        self.silent = silent
        self.commands = []
        self.stop = threading.Event()
        self.thread = threading.Thread(target=self.run)
        self.thread.start()

    def run(self):
        while not self.stop.is_set():
            if not select.select([self.master], [], [], 0.05)[0]:
                continue
            for command in os.read(self.master, 1024):
                self.commands.append(command)
                if command == ord("v") and not self.silent:
                    if self.stop.wait(self.delay):
                        return
                    # Deliver a fragmented welcome, including a gap between '$' characters.
                    os.write(self.master, b"OpenBCI PTY $")
                    if self.stop.wait(0.03):
                        return
                    os.write(self.master, b"$$")
                elif command == ord("d"):
                    os.write(self.master, b"defaults loaded\n")

    def close(self):
        self.stop.set()
        self.thread.join(timeout=2)
        assert not self.thread.is_alive()
        os.close(self.master)
        os.close(self.slave)


def main(library_path):
    library = ctypes.CDLL(os.path.abspath(library_path))
    for name in ("prepare_session", "release_session"):
        function = getattr(library, name)
        function.argtypes = [ctypes.c_int, ctypes.c_char_p]
        function.restype = ctypes.c_int
    library.is_prepared.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_char_p]
    library.is_prepared.restype = ctypes.c_int

    # This serial filter must never fall back to opening any installed adapter.
    assert library.prepare_session(0, params(serial_number="__brainflow_missing_test_device__")) == 13
    for timeout in (-1, 601):
        assert library.prepare_session(0, params(serial_port="/missing", timeout=timeout)) == 13
    assert library.prepare_session(0, params(serial_port="/brainflow_nonexistent_port")) == 2

    devices = [Dongle(delay=0.2), Dongle(delay=0.1)]
    configurations = [params(serial_port=device.name, serial_number="ignored_with_explicit_port")
                      for device in devices]
    try:
        for board_id, configuration in zip((0, 2), configurations):
            assert library.prepare_session(board_id, configuration) == 0
            prepared = ctypes.c_int()
            assert library.is_prepared(ctypes.byref(prepared), board_id, configuration) == 0
            assert prepared.value == 1
        for board_id, device, configuration in zip((0, 2), devices, configurations):
            assert library.release_session(board_id, configuration) == 0
            assert device.commands[:2] == [ord("v"), ord("d")]
            # Reopen proves release and the original session key still work.
            assert library.prepare_session(board_id, configuration) == 0
            assert library.release_session(board_id, configuration) == 0
    finally:
        library.release_all_sessions()
        for device in devices:
            device.close()

    device = Dongle(silent=True)
    configuration = params(serial_port=device.name, timeout=1)
    try:
        start = time.monotonic()
        assert library.prepare_session(0, configuration) == 7
        elapsed = time.monotonic() - start
        assert 0.9 <= elapsed < 3, elapsed
        # Failure must not leave a prepared session or an unusable port.
        device.silent = False
        assert library.prepare_session(0, configuration) == 0
        assert library.release_session(0, configuration) == 0
    finally:
        library.release_all_sessions()
        device.close()
    print("OpenBCI PTY integration tests passed")


if __name__ == "__main__":
    main(sys.argv[1])
