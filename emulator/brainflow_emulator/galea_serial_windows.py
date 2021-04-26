import os
import sys
import subprocess
import logging
import pkg_resources
import time
import threading
import random
import struct

from serial import Serial

from brainflow_emulator.emulate_common import TestFailureError, log_multilines


def write(port, data):
    return port.write(data)


def read(port, num_bytes):
    return port.read(num_bytes)


def get_isntaller():
    return pkg_resources.resource_filename(__name__, os.path.join('com0com', 'setup_com0com_W7_x64_signed.exe'))


def install_com0com():
    this_directory = os.path.abspath(os.path.dirname(__file__))
    directory = os.path.join(this_directory, 'com0com')
    if not os.path.exists(directory):
        os.makedirs(directory)
    cmds = [get_isntaller(), '/NCRC', '/S', '/D=%s' % directory]
    logging.info('running %s' % ' '.join(cmds))
    p = subprocess.Popen(cmds, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    if p.returncode != 0:
        logging.error('stdout is %s' % out)
        logging.error('stderr is %s' % err)
        raise Exception('com0com installation failure')
    logging.info('Sleeping a few second, it doesnt work in appveyour without it')
    time.sleep(10)
    return directory


def get_ports_windows():
    directory = install_com0com()
    # remove ports from previous run if any
    p = subprocess.Popen([os.path.join(directory, 'setupc.exe'), 'remove', '0'],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=directory)
    stdout, stderr = p.communicate()
    logging.info('remove stdout is %s' % stdout)
    logging.info('remove stderr is %s' % stderr)

    m_name = 'COM16'
    s_name = 'COM17'

    p = subprocess.Popen(
        [os.path.join(directory, 'setupc.exe'), 'install', 'PortName=%s' % m_name, 'PortName=%s' % s_name],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=directory)
    stdout, stderr = p.communicate()
    logging.info('install stdout is %s' % stdout)
    logging.info('install stderr is %s' % stderr)

    if p.returncode != 0:
        raise Exception('com0com failure')
    logging.info('Sleeping a few second, it doesnt work in appveyour without it')
    time.sleep(10)
    return m_name, s_name


def test_serial(cmd_list, m_name, s_name):
    master = Serial('\\\\.\\%s' % m_name, timeout=0)
    listen_thread = Listener(master, write, read)
    listen_thread.daemon = True
    listen_thread.start()

    cmd_to_run = cmd_list + [s_name]
    logging.info('Running %s' % ' '.join([str(x) for x in cmd_to_run]))
    process = subprocess.Popen(cmd_to_run, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()

    log_multilines(logging.info, stdout)
    log_multilines(logging.info, stderr)

    master.close()
    if process.returncode != 0:
        raise TestFailureError('Test failed with exit code %s' % str(process.returncode), process.returncode)

    return stdout, stderr


class Listener(threading.Thread):

    def __init__(self, port, write, read):
        # for windows write and read are methods from Serial object, for linux - os.read/write it doesnt work otherwise
        threading.Thread.__init__(self)
        self.port = port
        self.writer_process = None
        self.write = write
        self.read = read

    def run(self):
        start_time = time.time()
        while True:
            res = self.read(self.port, 9)
            if len(res) < 1:
                time.sleep(1)
                continue
            logging.info('read "%s"' % res)
            if 'F444' in res.decode('utf-8'):
                cur_time = time.time()
                resp = bytearray(struct.pack('d', (cur_time - start_time) * 1000))
                self.write(self.port, resp)
                logging.info('sent resp to calc time command')
            elif 'b' in res.decode('utf-8'):
                self.writer_process = GaleaWriter(self.port, 0.005, self.write)
                self.writer_process.daemon = True
                self.writer_process.start()
            elif 's' in res.decode('utf-8'):
                if self.writer_process is not None:
                    if self.writer_process.is_alive():
                        self.writer_process.need_data = False
                        self.writer_process.join()
            else:
                # we dont handle commands to turn on/off channels, gain signal and so on. such commands dont change package format
                logging.info('got command "%s"' % res)


class GaleaWriter(threading.Thread):

    def __init__(self, port, delay, write):
        threading.Thread.__init__(self)
        self.port = port
        self.write = write
        self.delay = delay
        self.package_size = 72 * 19
        self.package_num = 0
        self.need_data = True

    def run(self):
        while self.need_data:
            if self.package_num % 256 == 0:
                self.package_num = 0

            package = list()
            package.append(0xA0)
            for i in range(self.package_size):
                package.append(random.randint(0, 255))
            package.append(0xC0)
            logging.debug('package is %s' % ' '.join([str(x) for x in package]))
            self.write(self.port, bytes(package))

            self.package_num = self.package_num + 1
            time.sleep(self.delay)


def main(cmd_list):
    if not cmd_list:
        raise Exception('No command to execute')

    m_name, s_name = get_ports_windows()
    test_serial(cmd_list, m_name, s_name)


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main(sys.argv[1:])
