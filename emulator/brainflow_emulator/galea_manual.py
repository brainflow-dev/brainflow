import enum
import logging
import random
import socket
import struct
import time


class State(enum.Enum):
    wait = 'wait'
    stream = 'stream'


class Message(enum.Enum):
    start_stream = b'b'
    stop_stream = b's'
    ack_values = (b'd', b'~6', b'~5', b'o', b'F0')
    ack_from_device = b'A'
    time_calc_command = b'F4444444'


class GaleaEmulator(object):

    def __init__(self):
        self.local_ip = '127.0.0.1'
        self.local_port = 2390
        self.server_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
        self.server_socket.settimeout(
            0.1)  # decreases sampling rate significantly because it will wait for recv 0.1 sec but it's just a test
        self.server_socket.bind((self.local_ip, self.local_port))
        self.state = State.wait.value
        self.addr = None
        self.exg_package_num = 0
        self.aux_package_num = 0
        self.exg_package_size = 58
        self.aux_package_size = 26
        self.num_exg_packages_in_transaction = 20
        self.num_aux_packages_in_transaction = 4

    def run(self):
        start_time = time.time()
        while True:
            try:
                msg, self.addr = self.server_socket.recvfrom(128)
                if msg == Message.start_stream.value:
                    self.state = State.stream.value
                elif msg == Message.stop_stream.value:
                    self.state = State.wait.value
                elif msg in Message.ack_values.value or msg.decode('utf-8').startswith('x'):
                    self.server_socket.sendto(Message.ack_from_device.value, self.addr)
                elif msg == Message.time_calc_command.value:
                    cur_time = time.time()
                    resp = bytearray(struct.pack('d', (cur_time - start_time) * 1000))
                    self.server_socket.sendto(resp, self.addr)
                else:
                    if msg:
                        # we dont handle board config characters because they dont change package format
                        logging.warn('received unexpected string %s', str(msg))
            except socket.timeout:
                logging.debug('timeout for recv')

            if self.state == State.stream.value:
                package = list()
                # exg
                for _ in range(self.num_exg_packages_in_transaction):
                    package.append(0xA0)
                    package.append(self.exg_package_num)
                    self.exg_package_num = self.exg_package_num + 1
                    if self.exg_package_num % 256 == 0:
                        self.exg_package_num = 0
                    for i in range(1, self.exg_package_size - 10):
                        package.append(random.randint(0, 255))
                    cur_time = time.time()
                    timestamp = bytearray(struct.pack('d', (cur_time - start_time) * 1000))
                    package.extend(timestamp)
                    package.append(0xC0)
                try:
                    self.server_socket.sendto(bytes(package), self.addr)
                except socket.timeout:
                    logging.info('timeout for send')
                # aux
                package = list()
                for _ in range(self.num_aux_packages_in_transaction):
                    package.append(0xA1)
                    package.append(self.aux_package_num)
                    self.aux_package_num = self.aux_package_num + 1
                    if self.aux_package_num % 256 == 0:
                        self.aux_package_num = 0
                    for i in range(1, self.aux_package_size - 10):
                        package.append(random.randint(0, 255))
                    cur_time = time.time()
                    timestamp = bytearray(struct.pack('d', (cur_time - start_time) * 1000))
                    package.extend(timestamp)
                    package.append(0xC0)
                try:
                    self.server_socket.sendto(bytes(package), self.addr)
                except socket.timeout:
                    logging.info('timeout for send')


def main():
    emulator = GaleaEmulator()
    emulator.run()


if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO)
    main()
