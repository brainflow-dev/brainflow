import os
import sys
import threading
import subprocess
import logging
import time
from random import randint


class TestFailureError (Exception):
    def __init__ (self, message, exit_code):
        super (TestFailureError, self).__init__ (message)
        self.exit_code = exit_code

NEED_DATA = False

class Listener (threading.Thread):

    def __init__ (self, port, writer):
        threading.Thread.__init__ (self)
        self.port = port
        self.writer_process = None
        self.writer = writer

    def run (self):
        global NEED_DATA
        while True:
            res = os.read (self.port, 1)
            logging.info ('read "%s"' % res)
            if res == b'v':
                os.write (self.port, b'OpenBCI Emulator $$$')
            elif res == b'b':
                NEED_DATA = True
                self.writer_process = self.writer (self.port, 0.005)
                self.writer_process.daemon = True
                self.writer_process.start ()
            elif res == b's':
                NEED_DATA = False
                self.writer_process.join ()
            else:
                logging.warning ('got unexpected command "%s"' % res)


class CythonWriter (threading.Thread):

    def __init__ (self, port, delay):
        self.port = port
        threading.Thread.__init__ (self)
        self.delay = delay
        self.package_size = 33
        self.package_num = 0

    def run (self):
        while NEED_DATA:
            if self.package_num % 256 == 0:
                self.package_num = 0
            package = list ()
            package.append (0xA0)
            package.append (self.package_num)
            for i in range (2, self.package_size - 1):
                package.append (randint (0, 255))
            package.append (0xC0)
            logging.debug ('package is %s' % ' '.join ([str (x) for x in package]))
            os.write (self.port, bytes (package))

            self.package_num = self.package_num + 1
            time.sleep (self.delay)


def test_serial (cmd_list, master, slave, m_name, s_name):
    cmd_to_run = cmd_list + [s_name]
    logging.info ('Running %s' % ' '.join ([str (x) for x in cmd_to_run]))

    listen_thread = Listener (master, CythonWriter)
    listen_thread.daemon = True
    listen_thread.start ()

    process = subprocess.Popen (cmd_to_run, stdout = subprocess.PIPE, stderr = subprocess.PIPE)

    stdout, stderr = process.communicate ()
    logging.info ('stdout is: %s' % stdout)
    logging.info ('stderr is: %s' % stderr)

    if process.returncode != 0:
        raise TestFailureError ('Test failed with exit code %s' % str (process.returncode), process.returncode)

    return stdout, stderr

