import datetime
import enum
import json
import logging
import random
import socket
import struct
import subprocess
import sys
import threading
import time

from brainflow_emulator.emulate_common import TestFailureError, log_multilines

BIOLISTENER_DATA_CHANNELS_COUNT = 8

BIOLISTENER_DATA_PACKET_DEBUG = 0
BIOLISTENER_DATA_PACKET_BIOSIGNALS = 1
BIOLISTENER_DATA_PACKET_IMU = 2

ADC_USED = 0  # ADS131M08


class DataPacket:
    FORMAT_STRING = f'=1B 1I 1B 1I 1B {BIOLISTENER_DATA_CHANNELS_COUNT}I 1B'

    def __init__(self, ts, tp, n, s_id, data):
        self.header = 0xA0
        self.ts = ts
        self.type = tp
        self.n = n
        self.s_id = s_id
        self.data = data
        self.footer = 0xC0

    def pack(self):
        return struct.pack(self.FORMAT_STRING, self.header, self.ts, self.type, self.n, self.s_id, *self.data, self.footer)

    @classmethod
    def unpack(cls, packed_data):
        format_string = cls.FORMAT_STRING
        unpacked_data = struct.unpack(format_string, packed_data)
        return cls(*unpacked_data)

    def __repr__(self):
        return (f'DataPacket(header={self.header}, ts={self.ts}, type={self.type}, '
                f'n={self.n}, s_id={self.s_id}, data={self.data}, footer={self.footer})')


class State(enum.Enum):
    wait = 'wait'
    stream = 'stream'


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
    thread = BioListenerEmulator()
    thread.start()
    return thread


class BioListenerEmulator(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self.local_ip = '127.0.0.1'
        self.local_port = 12345
        self.server_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.settimeout(1)
        self.state = State.wait.value
        self.package_num = 0
        self.keep_alive = True
        self.connection_established = False
        logging.info(f"BioListener emulator started")

    @staticmethod
    def volts_to_data(ref, voltage, pga_gain, adc_resolution):
        resolution = ref / (adc_resolution * pga_gain)

        if voltage >= 0:  # Positive range
            raw_code = voltage / resolution
        else:  # Negative range
            raw_code = (voltage + (ref / pga_gain)) / resolution

        raw_code = int(raw_code)
        raw_code = max(0, min(0xFFFFFF, raw_code))  # Ensure within 24 bit range

        return raw_code

    def run(self):
        logging.info(f"BioListener emulator connecting to {self.local_ip}:{self.local_port}...")
        while self.keep_alive and not self.connection_established:
            try:
                self.server_socket.connect((self.local_ip, self.local_port))
                self.connection_established = True
                break
            except Exception as err:
                logging.warning(f"Error connecting to {self.local_ip}:{self.local_port}: {err}")
                # A failed connect may leave the socket unusable.
                try:
                    self.server_socket.close()
                except Exception:
                    pass
                # Recreate the socket with the same options.
                self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                self.server_socket.settimeout(1)
                time.sleep(0.1)

        if self.connection_established:
            logging.info(f"BioListener emulator connected to {self.local_ip}:{self.local_port}")
        else:
            logging.error(f"BioListener emulator failed to connect to {self.local_ip}:{self.local_port}")
            return

        started_at = time.time()
        while self.keep_alive:
            new_data_packet = DataPacket(
                ts=int((time.time() - started_at) * 1000),
                tp=BIOLISTENER_DATA_PACKET_BIOSIGNALS,
                n=self.package_num,
                s_id=ADC_USED,
                data=[
                    self.volts_to_data(
                        ref=2500000.0,
                        voltage=random.uniform(-1000, 1000),
                        pga_gain=8,
                        adc_resolution=16777216.0
                    ) for _ in range(BIOLISTENER_DATA_CHANNELS_COUNT)
                ]
            )
            self.package_num += 1

            try:
                data = self.server_socket.recv(1024)
                message = data.decode('utf-8').strip()

                if message:
                    for message_part in message.split("\n"):
                        logging.info(f"BioListener received command: {message_part}")
                        json_str = json.loads(message_part)
                        if json_str["command"] in (1, 2, 3, 4):
                            logging.info("Command ignored - simulator supports only start and stop stream command")
                        elif json_str["command"] == 5:
                            logging.info("Start stream command received")
                            self.state = State.stream.value
                        elif json_str["command"] == 6:
                            logging.info("Stop stream command received")
                            self.state = State.wait.value
                        else:
                            logging.warning(f"Unknown command: {json_str['command']}")
            except TimeoutError:
                pass
            except socket.timeout:
                pass
            except Exception as err:
                logging.error(f"Error in recv thread: {err}")

            try:
                if self.state == State.stream.value:
                    self.server_socket.sendall(new_data_packet.pack())
            except ConnectionResetError:
                logging.error(f"Connection lost")
            except Exception as e:
                logging.error(f"Error: {e}")


def main(cmd_list):
    if not cmd_list:
        raise Exception('No command to execute')
    server_thread = run_socket_server()

    try:
        test_socket(cmd_list)
    finally:
        server_thread.keep_alive = False
        server_thread.join()


if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO)
    main(sys.argv[1:])
