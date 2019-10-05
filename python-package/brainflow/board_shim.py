import ctypes
import numpy
from numpy.ctypeslib import ndpointer
import pkg_resources
import enum
import os
import platform
import sys
import struct
from brainflow.exit_codes import StreamExitCodes


class BrainFlowError (Exception):
    """This exception is raised if non-zero exit code is returned from C code"""
    def __init__ (self, message, exit_code):
        detailed_message = '%s:%d %s' % (StreamExitCodes (exit_code).name, exit_code, message)
        super (BrainFlowError, self).__init__ (detailed_message)
        self.exit_code = exit_code


class CYTON (object):
    board_id = 0
    fs_hz = 250
    num_eeg_channels = 8
    package_length = 12


class GANGLION (object):
    board_id = 1
    fs_hz = 200
    num_eeg_channels = 4
    package_length = 8


class SYNTHETIC (object):
    board_id = -1
    fs_hz = 256
    num_eeg_channels = 8
    package_length = 12


class CYTON_DAISY (object):
    board_id = 2
    fs_hz = 125
    num_eeg_channels = 16
    package_length = 20


class NOVAXR (object):
    board_id = 3
    fs_hz = 2000
    num_eeg_channels = 16
    package_length = 25


class GANGLION_WIFI (object):
    board_id = 4
    fs_hz = 1600
    num_eeg_channels = 4
    package_length = 8


class CYTON_WIFI (object):
    board_id = 5
    fs_hz = 1000
    num_eeg_channels = 8
    package_length = 12


class CYTON_DAISY_WIFI (object):
    board_id = 6
    fs_hz = 1000
    num_eeg_channels = 16
    package_length = 20


class BoardInfoGetter (object):
    """class to get information about boards, it's recommended to use this class instead hardcoded values"""

    @classmethod
    def get_fs_hz (cls, board_id):
        """Get sampling rate"""
        if board_id == CYTON.board_id:
            return CYTON.fs_hz
        elif board_id == GANGLION.board_id:
            return GANGLION.fs_hz
        elif board_id == SYNTHETIC.board_id:
            return SYNTHETIC.fs_hz
        elif board_id == CYTON_DAISY.board_id:
            return CYTON_DAISY.fs_hz
        elif board_id == NOVAXR.board_id:
            return NOVAXR.fs_hz
        elif board_id == CYTON_WIFI.board_id:
            return CYTON_WIFI.fs_hz
        elif board_id == CYTON_DAISY_WIFI.board_id:
            return CYTON_DAISY_WIFI.fs_hz
        elif board_id == GANGLION_WIFI.board_id:
            return GANGLION_WIFI.fs_hz
        else:
            raise BrainFlowError ('unsupported board type', StreamExitCodes.UNSUPPORTED_BOARD_ERROR.value)

    @classmethod
    def get_num_eeg_channels (cls, board_id):
        """Get number of eeg channels"""
        if board_id == CYTON.board_id:
            return CYTON.num_eeg_channels
        elif board_id == GANGLION.board_id:
            return GANGLION.num_eeg_channels
        elif board_id == SYNTHETIC.board_id:
            return SYNTHETIC.num_eeg_channels
        elif board_id == CYTON_DAISY.board_id:
            return CYTON_DAISY.num_eeg_channels
        elif board_id == NOVAXR.board_id:
            return NOVAXR.num_eeg_channels
        elif board_id == CYTON_WIFI.board_id:
            return CYTON_WIFI.num_eeg_channels
        elif board_id == CYTON_DAISY_WIFI.board_id:
            return CYTON_DAISY_WIFI.num_eeg_channels
        elif board_id == GANGLION_WIFI.board_id:
            return GANGLION_WIFI.num_eeg_channels
        else:
            raise BrainFlowError ('unsupported board type', StreamExitCodes.UNSUPPORTED_BOARD_ERROR.value)

    @classmethod
    def get_package_length (cls, board_id):
        """Get package length"""
        if board_id == CYTON.board_id:
            return CYTON.package_length
        elif board_id == GANGLION.board_id:
            return GANGLION.package_length
        elif board_id == SYNTHETIC.board_id:
            return SYNTHETIC.package_length
        elif board_id == CYTON_DAISY.board_id:
            return CYTON_DAISY.package_length
        elif board_id == NOVAXR.board_id:
            return NOVAXR.package_length
        elif board_id == CYTON_WIFI.board_id:
            return CYTON_WIFI.package_length
        elif board_id == CYTON_DAISY_WIFI.board_id:
            return CYTON_DAISY_WIFI.package_length
        elif board_id == GANGLION_WIFI.board_id:
            return GANGLION_WIFI.package_length
        else:
            raise BrainFlowError ('unsupported board type', StreamExitCodes.UNSUPPORTED_BOARD_ERROR.value)


class BoardControllerDLL (object):

    __instance = None

    @classmethod
    def get_instance (cls):
        if cls.__instance is None:
            cls.__instance = cls ()
        return cls.__instance

    def __init__ (self):
        if platform.system () == 'Windows':
            if struct.calcsize ("P") * 8 == 64:
                dll_path = 'lib\\BoardController.dll'
            else:
                dll_path = 'lib\\BoardController32.dll'
        elif platform.system () == 'Darwin':
            dll_path = 'lib/libBoardController.dylib'
        else:
            dll_path = 'lib/libBoardController.so'
        full_path = pkg_resources.resource_filename (__name__, dll_path)
        if os.path.isfile (full_path):
            self.lib = ctypes.cdll.LoadLibrary (full_path)
        else:
            raise FileNotFoundError ('Dynamic library %s is missed, did you forget to compile brainflow before installation of python package?' % full_path)

        self.prepare_session = self.lib.prepare_session
        self.prepare_session.restype = ctypes.c_int
        self.prepare_session.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.start_stream = self.lib.start_stream
        self.start_stream.restype = ctypes.c_int
        self.start_stream.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.stop_stream = self.lib.stop_stream
        self.stop_stream.restype = ctypes.c_int
        self.stop_stream.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.get_current_board_data = self.lib.get_current_board_data
        self.get_current_board_data.restype = ctypes.c_int
        self.get_current_board_data.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_float),
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_int64),
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.get_board_data = self.lib.get_board_data
        self.get_board_data.restype = ctypes.c_int
        self.get_board_data.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_float),
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.release_session = self.lib.release_session
        self.release_session.restype = ctypes.c_int
        self.release_session.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.get_board_data_count = self.lib.get_board_data_count
        self.get_board_data_count.restype = ctypes.c_int
        self.get_board_data_count.argtypes = [
            ndpointer (ctypes.c_int64),
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.set_log_level = self.lib.set_log_level
        self.set_log_level.restype = ctypes.c_int
        self.set_log_level.argtypes = [
           ctypes.c_int
        ]

        self.set_log_file = self.lib.set_log_file
        self.set_log_file.restype = ctypes.c_int
        self.set_log_file.argtypes = [
            ctypes.c_char_p
        ]

        self.config_board = self.lib.config_board
        self.config_board.restype = ctypes.c_int
        self.config_board.argtypes = [
            ctypes.c_char_p,
            ctypes.c_int,
            ctypes.c_char_p
        ]


class BoardShim (object):
    """BoardShim class is a primary interface to all boards"""

    def __init__ (self, board_id, port_name):
        if port_name:
            # handle bytes and str objects
            try:
                self.port_name = port_name.encode ()
            except:
                self.port_name = port_name
        else:
            self.port_name = None
        self.board_id = board_id
        self.package_length = BoardInfoGetter.get_package_length (board_id)
        self.fs_hz = BoardInfoGetter.get_fs_hz (board_id)
        self.num_eeg_channels = BoardInfoGetter.get_num_eeg_channels (board_id)

    @classmethod
    def enable_board_logger (cls):
        """enable board logger"""
        res = BoardControllerDLL.get_instance ().set_log_level (2)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to enable logger', res)

    @classmethod
    def disable_board_logger (cls):
        """disable board logger"""
        res = BoardControllerDLL.get_instance ().set_log_level (6)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to disable logger', res)

    @classmethod
    def enable_dev_board_logger (cls):
        """enable it to check developers log messages"""
        res = BoardControllerDLL.get_instance ().set_log_level (0)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to enable logger', res)

    @classmethod
    def set_log_file (cls, log_file):
        """redirect logger from stderr to file, can be called any time"""
        try:
            file = log_file.encode ()
        except:
            file = log_file
        res = BoardControllerDLL.get_instance ().set_log_file (file)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to redirect logs to a file', res)

    def prepare_session (self):
        """prepare streaming sesssion, init resources, you need to call it before any other BoardShim object methods"""
        res = BoardControllerDLL.get_instance ().prepare_session (self.board_id, self.port_name)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to prepare streaming session', res)

    def start_stream (self, num_samples = 1800*250):
        """Start streaming data, this methods stores data in ringbuffer"""
        res = BoardControllerDLL.get_instance ().start_stream (num_samples, self.board_id, self.port_name)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to start streaming session', res)

    def stop_stream (self):
        """Stop streaming data"""
        res = BoardControllerDLL.get_instance ().stop_stream (self.board_id, self.port_name)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to stop streaming session', res)

    def release_session (self):
        """release all resources"""
        res = BoardControllerDLL.get_instance ().release_session (self.board_id, self.port_name)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to release streaming session', res)

    def get_current_board_data (self, num_samples):
        """Get specified amount of data or less if there is not enough data, doesnt remove data from ringbuffer"""
        data_arr = numpy.zeros (int(num_samples  * self.package_length)).astype (numpy.float32)
        time_arr = numpy.zeros (num_samples).astype (numpy.float64)
        current_size = numpy.zeros (1).astype (numpy.int64)

        res = BoardControllerDLL.get_instance ().get_current_board_data (num_samples, data_arr, time_arr, current_size, self.board_id, self.port_name)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to get current data', res)

        if len (current_size) == 0:
            return None

        data_arr = data_arr[0:current_size[0] * self.package_length].reshape (current_size[0], self.package_length)
        time_arr = time_arr[0:current_size[0]]
        return numpy.column_stack ((data_arr, time_arr))

    def get_immediate_board_data (self):
        """Get the latest package, doesnt remove data from ringbuffer"""
        return self.get_current_board_data (1)

    def get_board_data_count (self):
        """Get num of elements in ringbuffer"""
        data_size = numpy.zeros (1).astype (numpy.int64)

        res = BoardControllerDLL.get_instance ().get_board_data_count (data_size, self.board_id, self.port_name)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to obtain buffer size', res)
        return data_size[0]

    def get_board_data (self):
        """Get all board data and remove them from ringbuffer"""
        data_size = self.get_board_data_count ()
        time_arr = numpy.zeros (data_size).astype (numpy.float64)
        data_arr = numpy.zeros (data_size * self.package_length).astype (numpy.float32)

        res = BoardControllerDLL.get_instance ().get_board_data (data_size, data_arr, time_arr, self.board_id, self.port_name)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to get board data', res)

        data_arr = data_arr.reshape (data_size, self.package_length)
        return numpy.column_stack ((data_arr, time_arr))

    def config_board (self, config):
        """Use this method carefully and only if you understand what you are doing, do NOT use it to start or stop streaming""" 
        try:
            config_string = config.encode ()
        except:
            config_string = config

        res = BoardControllerDLL.get_instance ().config_board (config_string, self.board_id, self.port_name)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to config board', res)
