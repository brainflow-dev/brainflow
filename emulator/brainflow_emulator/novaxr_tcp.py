import subprocess
import threading
import socket
import enum
import logging
import sys
import random
import time
import struct

from brainflow_emulator.emulate_common import TestFailureError, log_multilines


class State (enum.Enum):
    wait = 'wait'
    stream = 'stream'


class Message (enum.Enum):
    start_stream = b'b'
    stop_stream = b's'


def test_socket (cmd_list):
    logging.info ('Running %s' % ' '.join ([str (x) for x in cmd_list]))
    process = subprocess.Popen (cmd_list, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    stdout, stderr = process.communicate ()

    log_multilines (logging.info, stdout)
    log_multilines (logging.info, stderr)

    if process.returncode != 0:
        raise TestFailureError ('Test failed with exit code %s' % str (process.returncode), process.returncode)

    return stdout, stderr


def run_socket_server ():
    novaxr_thread = NovaXREmulator ()
    novaxr_thread.start ()
    return novaxr_thread


class NovaXREmulator (threading.Thread):

    def __init__ (self):
        threading.Thread.__init__ (self)
        self.local_ip = '127.0.0.1'
        self.local_port = 2390
        self.server_socket = socket.socket (family = socket.AF_INET, type = socket.SOCK_STREAM)
        self.server_socket.settimeout (0.1)
        self.server_socket.bind ((self.local_ip, self.local_port))
        self.server_socket.listen (1)
        self.state = State.wait.value
        self.addr = None
        self.conn = None
        self.package_num = 0
        self.package_size = 72
        self.keep_alive = True

    def run (self):
        for i in range (50):
            try:
                self.conn, self.addr = self.server_socket.accept ()
                break
            except socket.timeout:
                pass
        if self.addr is None:
            raise TestFailureError ('failed to establish connection')

        while self.keep_alive:
            if self.package_num % 256 == 0:
                self.package_num = 0
            # dirtiest hack ever but it doesnt work otherwise. seems like recv ignores timeout and it means that we send only 1 package
            if self.package_num == 0:
                try:
                    msg = self.conn.recv (128)
                    if msg:
                        logging.info ('received %s' % (msg))
                        if msg == Message.start_stream.value:
                            self.state = State.stream.value
                        elif msg == Message.stop_stream.value:
                            self.state = State.wait.value
                        else:
                            # we dont handle board config characters because they dont change package format
                            logging.warn ('received unexpected string %s', str (msg))
                except socket.timeout:
                    logging.debug ('timeout for recv')

            if self.state == State.stream.value:
                package = list ()
                package.append (self.package_num)
                self.package_num = self.package_num + 1
                for i in range (1, self.package_size - 8):
                    package.append (random.randint (0, 255))
                timestamp = bytearray (struct.pack ("d", time.time ()))
                package.extend (timestamp)
                try:
                    self.conn.send (bytes (package))
                except socket.timeout:
                    logging.info ('timeout for send')

def main (cmd_list):
    if not cmd_list:
        raise Exception ('No command to execute')
    server_thread = run_socket_server ()
    test_socket (cmd_list)
    server_thread.keep_alive = False
    server_thread.join ()


if __name__=='__main__':
    logging.basicConfig (format = '%(asctime)s %(levelname)-8s %(message)s', level = logging.INFO)
    main (sys.argv[1:])
