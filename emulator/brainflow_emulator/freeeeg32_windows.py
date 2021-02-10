import os
import sys
import subprocess
import logging
import pkg_resources
import time

from serial import Serial

from brainflow_emulator.freeeeg32_emulator import Listener
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

    m_name = 'COM14'
    s_name = 'COM15'

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


def main(cmd_list):
    if not cmd_list:
        raise Exception('No command to execute')

    m_name, s_name = get_ports_windows()
    test_serial(cmd_list, m_name, s_name)


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main(sys.argv[1:])
