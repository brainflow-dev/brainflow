import logging
import threading
import time
from random import randint

# Import common emulator utilities
from brainflow_emulator.emulate_common import log_multilines, TestFailureError

class PIEEGWriter(threading.Thread):
    def __init__(self, port, write):
        threading.Thread.__init__(self)
        self.port = port
        self.write = write
        self.delay = 1
        self.package_size = 33
        self.package_num = 0
        self.need_data = True

    def run(self):
        while self.need_data:
            package = list()
            for _ in range(6):
                package.append(0xA0)
                package.append(self.package_num)
                for i in range(2, self.package_size - 1):
                    package.append(randint(0, 255))
                package.append(0xC0)
                self.package_num = self.package_num + 1
                if self.package_num % 256 == 0:
                    self.package_num = 0
            self.write(self.port, bytes(package))
            time.sleep(self.delay)

class PIEEGListener(threading.Thread):
    def __init__(self, port, write, read):
        threading.Thread.__init__(self)
        self.port = port
        self.writer_process = None
        self.write = write
        self.read = read

    def run(self):
        while True:
            res = self.read(self.port, 1)
            if len(res) < 1:
                time.sleep(1)
                continue
            logging.info('read "%s"' % res)
            if res == b'v':
                self.write(self.port, b'PIEEG Emulator $$$')
            elif res == b'?':
                self.write(self.port, b'Imagine registers here')
            elif res == b'b':
                self.writer_process = PIEEGWriter(self.port, self.write)
                self.writer_process.daemon = True
                self.writer_process.start()
            elif res == b's':
                if self.writer_process is not None:
                    if self.writer_process.is_alive():
                        self.writer_process.need_data = False
                        self.writer_process.join()
            else:
                logging.warning('got unexpected command "%s"' % res)

def start_emulator(port):
    try:
        from serial import Serial
        serial_port = Serial(port, baudrate=115200, timeout=1)
        listener = PIEEGListener(serial_port, serial_port.write, serial_port.read)
        listener.daemon = True
        listener.start()
        while True:
            time.sleep(1)
    except Exception as e:
        logging.error(f"Error starting emulator: {e}")

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    start_emulator("/dev/ttyUSB0")  # Adjust the port as necessary
