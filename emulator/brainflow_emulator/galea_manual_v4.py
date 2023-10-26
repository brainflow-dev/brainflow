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
        self.server_socket.bind((self.local_ip, self.local_port))
        self.state = State.wait.value
        self.addr = None
        self.package_num = 0
        self.transaction_size = 12
        self.package_size = 114
        self.sampling_rate = 250
        self.transaction_speed_250 = .048 - .003225 #.048 is the time it takes to send 12 packages at 250Hz, we have to adjust this with an arbitrary value
        self.server_socket.settimeout(
            .0000001)  # decreases sampling rate significantly because it will wait for receive to timeout
        self.total_packets_sent = 0
        self.start_streaming_time = 0
        self.DEBUG_MODE = False

    def run(self):
        start_time = time.time()
        while True:
            try:
                msg, self.addr = self.server_socket.recvfrom(128)
                if msg == Message.start_stream.value:
                    self.state = State.stream.value
                    self.start_streaming_time = time.time()
                elif msg == Message.stop_stream.value:
                    self.state = State.wait.value
                    self.total_packets_sent = 0
                    self.start_streaming_time
                elif msg in Message.ack_values.value or msg.decode('utf-8').startswith('x'):
                    self.server_socket.sendto(Message.ack_from_device.value, self.addr)
                elif msg in Message.ack_values.value or msg.decode('utf-8').startswith('z'):
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
                transaction = list()
                for _ in range(self.transaction_size):
                    single_package = list()
                    for i in range(self.package_size):
                        single_package.append(random.randint(0, 255))
                    single_package[0] = self.package_num

                    cur_time = time.time()
                    timestamp = bytearray(struct.pack('d', (cur_time - start_time) * 1000))
                    eda = bytearray(struct.pack('f', random.random()))
                    ppg_red = bytearray(struct.pack('i', int(random.random() * 5000)))
                    ppg_ir = bytearray(struct.pack('i', int(random.random() * 5000)))

                    for i in range(88, 96):
                        single_package[i] = timestamp[i - 88]
                    for i in range(1, 5):
                        single_package[i] = eda[i - 1]
                    for i in range(84, 88):
                        single_package[i] = ppg_ir[i - 84]
                    for i in range(80, 84):
                        single_package[i] = ppg_red[i - 80]
                    single_package[77] = random.randint(0, 100)

                    self.package_num = self.package_num + 1
                    if self.package_num % 256 == 0:
                        self.package_num = 0
                    transaction.append(single_package)
                try:
                    package = list()
                    for i in range(self.transaction_size):
                        package.extend(bytes(transaction[i]))
                    self.server_socket.sendto(bytes(package), self.addr)
                    self.total_packets_sent += self.transaction_size
                except socket.timeout:
                    logging.info('timeout for send')
                
                time.sleep(self.transaction_speed_250)

                if (self.DEBUG_MODE):
                    elapsed_time = (time.time() - self.start_streaming_time)
                    sampling_rate = self.total_packets_sent / elapsed_time
                    print('elapsed time: ' + str(elapsed_time) + ' sampling rate: ' + str(sampling_rate))

def main():
    emulator = GaleaEmulator()
    emulator.run()


if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO)
    main()