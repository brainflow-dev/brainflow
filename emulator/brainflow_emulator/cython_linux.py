import os
import sys
import pty
import logging

from brainflow_emulator.emulate_common import test_serial


def get_ports_pty ():
    master, slave = pty.openpty ()
    s_name = os.ttyname (slave)
    m_name = os.ttyname (master)
    return master, slave, m_name, s_name

def main (cmd_list):
    if not cmd_list:
        raise Exception ('No command to execute')
    master, slave, m_name, s_name = get_ports_pty ()
    test_serial (cmd_list, master, slave, m_name, s_name)


if __name__=='__main__':
    logging.basicConfig (level = logging.DEBUG)
    main (sys.argv[1:])
