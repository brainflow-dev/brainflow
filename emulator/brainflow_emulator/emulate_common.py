import threading
import logging
import time
from random import randint


def log_multilines(log_function, message):
    for line in message.splitlines():
        log_function(line)


class TestFailureError(Exception):
    def __init__(self, message, exit_code):
        super(TestFailureError, self).__init__(message)
        self.exit_code = exit_code


class Listener(threading.Thread):

    def __init__(self, port, write, read):
        # for windows write and read are methods from Serial object, for linux - os.read/write it doesnt work otherwise
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
                self.write(self.port, b'OpenBCI Emulator $$$')
            elif res == b'?':
                self.write(self.port, b'Imagine registers here')
            elif res == b'b':
                self.writer_process = CytonWriter(self.port, 0.005, self.write)
                self.writer_process.daemon = True
                self.writer_process.start()
            elif res == b's':
                if self.writer_process is not None:
                    if self.writer_process.is_alive():
                        self.writer_process.need_data = False
                        self.writer_process.join()
            else:
                # we dont handle commands to turn on/off channels, gain signal and so on. such commands dont change package format
                logging.warning('got unexpected command "%s"' % res)


class CytonWriter(threading.Thread):

    def __init__(self, port, delay, write):
        threading.Thread.__init__(self)
        self.port = port
        self.write = write
        self.delay = delay
        self.package_size = 33
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
                package.append(randint(0, 255))
            package.append(0xC0)
            logging.debug('package is %s' % ' '.join([str(x) for x in package]))
            self.write(self.port, bytes(package))

            self.package_num = self.package_num + 1
            time.sleep(self.delay)
