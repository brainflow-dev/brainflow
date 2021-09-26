import threading
import logging
import time
from random import randint


class Listener(threading.Thread):

    def __init__(self, port, write, read):
        # for windows write and read are methods from Serial object, for linux - os.read/write it doesnt work otherwise
        threading.Thread.__init__(self)
        self.port = port
        self.writer_process = None
        self.write = write
        self.read = read
        self.need_stop = False

    def run(self):
        self.writer_process = FreeEEG32Writer(self.port, 0.005, self.write)
        self.writer_process.daemon = True
        self.writer_process.start()
        time.sleep(10)
        self.writer_process.need_data = False
        self.writer_process.join()


class FreeEEG32Writer(threading.Thread):

    def __init__(self, port, delay, write):
        threading.Thread.__init__(self)
        self.port = port
        self.write = write
        self.delay = delay
        self.package_size = 106
        self.package_num = 0
        self.need_data = True

    def run(self):
        while self.need_data:
            if self.package_num % 256 == 0:
                self.package_num = 0

            package = list()
            package.append(0xA0)
            package.append(self.package_num)
            for i in range(2, self.package_size - 1):
                package.append(i)
            package.append(0xC0)
            logging.info(bytes(package))
            self.write(self.port, bytes(package))

            self.package_num = self.package_num + 1
            time.sleep(self.delay)
