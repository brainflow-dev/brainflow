import socket
import enum
import logging
import sys
import random
import time
import struct


class FasciaEmulator():

    def __init__(self, ip, port):
        self.server_ip = ip
        self.server_port = int(port)
        logging.info('ip %s port %d' % (self.server_ip, self.server_port))
        self.server_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
        self.server_socket.settimeout(0.1)
        self.package_num = 0
        self.package_size = 68
        self.package_in_transaction = 20

    def run(self):
        while True:
            package = list()
            for _ in range(self.package_in_transaction):
                package.extend(bytearray(struct.pack("i", self.package_num)))
                self.package_num = self.package_num + 1
                for i in range(self.package_size - 4):
                    package.append(random.randint(0, 255))
            try:
                self.server_socket.sendto(bytes(package), (self.server_ip, self.server_port))
            except socket.timeout:
                logging.info('timeout for send')

            time.sleep(0.1)


def main():
    emulator = FasciaEmulator(sys.argv[1], sys.argv[2])
    emulator.run()


if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO)
    main()
