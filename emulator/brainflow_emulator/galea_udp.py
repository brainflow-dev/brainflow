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


class State(enum.Enum):
    wait = 'wait'
    stream = 'stream'


class Message(enum.Enum):
    start_stream = b'b'
    stop_stream = b's'
    ack_values = (b'd', b'~6', b'~5', b'o', b'F0')
    ack_from_device = b'A'
    time_calc_command = b'F4444444'


def test_socket(cmd_list):
    logging.info('Running %s' % ' '.join([str(x) for x in cmd_list]))
    process = subprocess.Popen(cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()

    log_multilines(logging.info, stdout)
    log_multilines(logging.info, stderr)

    if process.returncode != 0:
        raise TestFailureError('Test failed with exit code %s' % str(process.returncode), process.returncode)

    return stdout, stderr


def run_socket_server():
    novaxr_thread = GaleaEmulator()
    novaxr_thread.start()
    return novaxr_thread


class GaleaEmulator(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self.local_ip = '127.0.0.1'
        self.local_port = 2390
        self.server_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
        self.server_socket.settimeout(
            0.1)  # decreases sampling rate significantly because it will wait for recv 0.1 sec but it's just a test
        self.server_socket.bind((self.local_ip, self.local_port))
        self.state = State.wait.value
        self.addr = None
        self.package_num = 0
        self.package_size = 72
        self.keep_alive = True


    def run (self):
        start_time = time.time ()
        while self.keep_alive:
            try:
                msg, self.addr = self.server_socket.recvfrom(128)
                if msg == Message.start_stream.value:
                    self.state = State.stream.value
                elif msg == Message.stop_stream.value:
                    self.state = State.wait.value
                elif msg in Message.ack_values.value:
                    self.server_socket.sendto(Message.ack_from_device.value, self.addr)
                elif msg == Message.time_calc_command.value:
                    cur_time = time.time ()
                    resp = bytearray (struct.pack ('d', (cur_time - start_time) * 1000))
                    self.server_socket.sendto (resp, self.addr)
                else:
                    if msg:
                        # we dont handle board config characters because they dont change package format
                        logging.warn('received unexpected string %s', str(msg))
            except socket.timeout:
                logging.debug('timeout for recv')
            except Exception:
                break

            if self.state == State.stream.value:
                package = list ()
                for _ in range (19):
                    package.append (self.package_num)
                    self.package_num = self.package_num + 1
                    if self.package_num % 256 == 0:
                        self.package_num = 0
                    for i in range (1, self.package_size - 8):
                        package.append (random.randint (0, 255))
                    cur_time = time.time ()
                    timestamp = bytearray (struct.pack ('d', (cur_time - start_time) * 1000))
                    package.extend (timestamp)
                try:
                    self.server_socket.sendto(bytes(package), self.addr)
                except socket.timeout:
                    logging.info ('timeout for send')


def main(cmd_list):
    if not cmd_list:
        raise Exception('No command to execute')
    server_thread = run_socket_server()
    test_socket(cmd_list)
    server_thread.keep_alive = False
    server_thread.join()


if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO)
    main(sys.argv[1:])
