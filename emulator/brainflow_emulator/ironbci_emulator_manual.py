import socket
import enum
import logging
import sys
import random
import time
import struct


class State (enum.Enum):
    wait = 'wait'
    stream = 'stream'


class IronBCIEmulator ():

    def __init__ (self, ip):
        self.ip = ip
        self.tcp_port = 2321
        self.udp_port = 2322
        self.tcp_socket = socket.socket (family = socket.AF_INET, type = socket.SOCK_STREAM)
        self.udp_socket = socket.socket (family = socket.AF_INET, type = socket.SOCK_DGRAM)
        self.tcp_socket.bind ((self.ip, self.tcp_port))
        self.tcp_socket.listen ()
        self.udp_socket.bind ((self.ip, self.udp_port))
        print ('waiting for the socket to connect')
        self.tcp_conn, self.client_addr = self.tcp_socket.accept ()
        print ('client addr is %s' % str (self.client_addr))
        self.tcp_socket.settimeout (0.1)
        self.tcp_conn.settimeout (0.1)
        self.package_num = 0
        self.package_size = 25
        self.packages_in_transaction = 10
        self.state = State.wait.value
        self.udp_client_port = 0

    def run (self):
        while True:
            try:
                msg = self.tcp_conn.recv (128)
                msg = msg.decode ('utf-8')
                if msg.startswith ('b'):
                    self.state = State.stream.value
                    self.udp_client_port = int (msg[1:])
                elif msg.startswith ('s'):
                    self.state = State.wait.value
                else:
                    if msg:
                        logging.warn ('received unexpected string %s', str (msg))
            except socket.timeout:
                pass

            if self.state == State.stream.value:
                package = list ()
                for _ in range (self.packages_in_transaction):
                    package.append (self.package_num)
                    self.package_num = self.package_num + 1
                    if self.package_num % 256 == 0:
                        self.package_num = 0
                    for i in range (1, self.package_size):
                        package.append (random.randint (0, 255))
                try:
                    dest = (self.client_addr[0], self.udp_client_port)
                    self.udp_socket.sendto (bytes (package), dest)
                    print ('sent package to %s' % str (dest))
                except socket.timeout:
                    logging.info ('timeout for send')


def main ():
    emulator = IronBCIEmulator(sys.argv[1])
    emulator.run ()


if __name__=='__main__':
    logging.basicConfig (format = '%(asctime)s %(levelname)-8s %(message)s', level = logging.INFO)
    main ()
