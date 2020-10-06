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


class NovaXREmulator (object):

    def __init__ (self, ip = '127.0.0.1'):
        self.ip = ip
        self.tcp_port = 2391
        self.udp_port = 2390
        self.package_num = 0
        self.package_size = 72
        self.state = State.wait.value
        self.udp_client_port = 0
        self.transaction_size = 19

    def init_tcp_socket (self):
        self.tcp_socket = socket.socket (family = socket.AF_INET, type = socket.SOCK_STREAM)
        self.tcp_socket.bind ((self.ip, self.tcp_port))
        self.tcp_socket.listen ()
        print ('waiting for the socket to connect')
        self.tcp_conn, self.client_addr = self.tcp_socket.accept ()
        print ('client addr is %s' % str (self.client_addr))
        self.tcp_socket.settimeout (0.1)
        self.tcp_conn.settimeout (0.1)

    def run (self):
        self.init_tcp_socket ()
        while True:
            try:
                msg = self.tcp_conn.recv (128)
                if len (msg) == 0:
                    # it means closed connection - reinitialize connection
                    self.init_tcp_socket ()
                    continue
                msg = msg.decode ('utf-8')
                if msg:
                    for command in msg.split ('\n'):
                        if command.startswith ('b'):
                            self.state = State.stream.value
                            self.udp_socket = socket.socket (family = socket.AF_INET, type = socket.SOCK_DGRAM)
                            self.udp_socket.bind ((self.ip, self.udp_port))
                            self.udp_socket.settimeout (0.1)
                            self.udp_client_port = int (command[1:])
                            print ('creating socket')
                        elif command.startswith ('s'):
                            self.udp_socket.close ()
                            self.state = State.wait.value
                            print ('closing socket')
                        else:
                            if command:
                                logging.warn ('received unexpected string %s', str (command))
            except socket.timeout:
                pass
            except socket.error as e:
                print (e)

            if self.state == State.stream.value:
                transaction = list ()
                for _ in range (self.transaction_size):
                    single_package = list ()
                    for i in range (self.package_size):
                        single_package.append (random.randint (0, 255))
                    single_package[0] = self.package_num
                    
                    timestamp = bytearray (struct.pack ('d', time.time ()))
                    eda = bytearray (struct.pack ('f', random.random ()))
                    ppg_red = bytearray (struct.pack ('i', int (random.random () * 5000)))
                    ppg_ir = bytearray (struct.pack ('i', int (random.random () * 5000)))
                    battery = bytearray (struct.pack ('i', int (random.random () * 100)))

                    for i in range (64, 72):
                        single_package[i] = timestamp[i - 64]
                    for i in range (1, 5):
                        single_package[i] = eda[i - 1]
                    for i in range (60, 64):
                        single_package[i] = ppg_ir[i - 60]
                    for i in range (56, 60):
                        single_package[i] = ppg_red[i - 56]
                    single_package[53] = random.randint (0, 100)

                    self.package_num = self.package_num + 1
                    if self.package_num % 256 == 0:
                        self.package_num = 0

                    transaction.append (single_package)
                try:
                    package = list ()
                    for i in range (self.transaction_size):
                        package.extend (bytes (transaction[i]))
                    dest = (self.client_addr[0], self.udp_client_port)
                    self.udp_socket.sendto (bytes (package), dest)
                except socket.timeout:
                    logging.info ('timeout for send')


def main ():
    emulator = NovaXREmulator ()
    emulator.run ()


if __name__=='__main__':
    logging.basicConfig (format = '%(asctime)s %(levelname)-8s %(message)s', level = logging.INFO)
    main ()
