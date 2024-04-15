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
    ack_values = (b'd', b'~6', b'~5', b'~4', b'o', b'F0')
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
        self.server_socket.settimeout(.0001)
        self.channel_on_off = [1] * 24
        self.channel_identifiers = [
            '1', '2', '3', '4', '5', '6', '7', '8',
            'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 
            'A', 'S', 'D', 'G', 'H', 'J', 'K', 'L'
        ]

    def run(self):
        start_time = time.time()
        while True:
            try:
                msg, self.addr = self.server_socket.recvfrom(128)
                if msg == Message.start_stream.value:
                    self.state = State.stream.value
                elif msg == Message.stop_stream.value:
                    self.state = State.wait.value
                elif msg.decode('utf-8').startswith('~'):
                    self.server_socket.sendto(Message.ack_from_device.value, self.addr)
                    self.process_sampling_rate(msg.decode('utf-8'))
                elif msg in Message.ack_values.value or msg.decode('utf-8').startswith('x'):
                    self.server_socket.sendto(Message.ack_from_device.value, self.addr)
                    self.process_channel_on_off(msg.decode('utf-8'))
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
                for t in range(self.transaction_size):
                    single_package = list()
                    channel = 0
                    for i in range(self.package_size):
                        if (i > 4 and i < 77):
                            sample = i - 4
                            if (sample % 3 == 1):
                                channel += 1
                            if (self.channel_on_off[channel - 1] == 1):
                                if (sample % 3 == 2):
                                    single_package.append(random.randint(0, 8 + (channel * 2)))
                                else:
                                    single_package.append(0)
                            else:
                               single_package.append(0)
                        else:
                            single_package.append(random.randint(0, 255))
                    single_package[0] = self.package_num

                    cur_time = time.time()
                    timestamp = bytearray(struct.pack('d', (cur_time - start_time) * 1000))
                    eda = bytearray(struct.pack('f', .5))
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
                except socket.timeout:
                    logging.info('timeout for send')
                time.sleep(0.001)


    def process_channel_on_off(self, msg):
        if msg.startswith('x'):
            for channel_id in self.channel_identifiers:
                channel_num = self.channel_identifiers.index(channel_id)
                if (msg[1] == channel_id):
                    if (msg[2] == '0'): # 0 is off (or Power Down), 1 is on
                        self.channel_on_off[channel_num] = 1
                        logging.info('channel '+ str(channel_num + 1) + ' is on')
                    else:
                        self.channel_on_off[channel_num] = 0
                        logging.info('channel ' + str(channel_num + 1) + ' is off')

    def process_sampling_rate(self, msg):
        if (msg[1] == '6'):
            logging.info('sampling rate is 250Hz')
        elif (msg[1] == '5'):
            logging.info('sampling rate is 500Hz')
        elif (msg[1] == '4'):
            logging.info('sampling rate is 1000Hz')
        else:
            logging.warning(f'did not recognize sampling rate command: {msg}')

def main():
    emulator = GaleaEmulator()
    emulator.run()


if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO)
    main()