import os
import sys
import subprocess
import logging
import pkg_resources
import atexit


from brainflow_emulator.emulate_common import test_serial


def get_pkg_dir ():
    return os.path.abspath (os.path.dirname (pkg_resources.resource_filename (__name__, 'com0com\\setupc.exe')))

def get_ports_windows ():
    # remove ports from previous run if any
    p = subprocess.Popen ([os.path.join (get_pkg_dir (), 'setupc.exe'), 'remove', '0'],
                         stdout = subprocess.PIPE, stderr = subprocess.PIPE, cwd = get_pkg_dir ())
    stdout, stderr = p.communicate ()
    logging.info ('remove stdout is %s' % stdout)
    logging.info ('remove stderr is %s' % stderr)

    # hopefully hardcoded values are good enough for emulators and tests
    m_name = 'COM8'
    s_name = 'COM9'

    p = subprocess.Popen ([os.path.join (get_pkg_dir (), 'setupc.exe'), 'install', 'PortName=%s' % m_name, 'PortName=%s' % s_name],
                        stdout = subprocess.PIPE, stderr = subprocess.PIPE, cwd = get_pkg_dir ())
    stdout, stderr = p.communicate ()
    logging.info ('install stdout is %s' % stdout)
    logging.info ('install stderr is %s' % stderr)

    if p.returncode != 0:
        raise Exception ('com0com failure')

    return os.open (m_name, os.O_RDWR), None, m_name, s_name

def main (cmd_list):
    if not cmd_list:
        raise Exception ('No command to execute')

    master, slave, m_name, s_name = get_ports_windows ()
    test_serial (cmd_list, master, slave, m_name, s_name)


if __name__=='__main__':
    logging.basicConfig (level = logging.INFO)
    main (sys.argv[1:])
