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


class BoardInfoGetter (object):

    @classmethod
    def get_fs_hz (cls, board_id):
        if board_id == CYTON.board_id:
            return CYTON.fs_hz
        elif board_id == GANGLION.board_id:
            return GANGLION.fs_hz
        else:
            raise BrainFlowError ('unsupported board type', StreamExitCodes.UNSUPPORTED_BOARD_ERROR.value)


    @classmethod
    def get_num_eeg_channels (cls, board_id):
        if board_id == CYTON.board_id:
            return CYTON.num_eeg_channels
        elif board_id == GANGLION.board_id:
            return GANGLION.num_eeg_channels
        else:
            raise BrainFlowError ('unsupported board type', StreamExitCodes.UNSUPPORTED_BOARD_ERROR.value)


    @classmethod
    def get_package_length (cls, board_id):
        if board_id == CYTON.board_id:
            return CYTON.package_length
        elif board_id == GANGLION.board_id:
            return GANGLION.package_length
        else:
            raise BrainFlowError ('unsupported board type', StreamExitCodes.UNSUPPORTED_BOARD_ERROR.value)


class BoardControllerDLL (object):

    __instance = None

    @classmethod
    def get_instance (cls):
        if cls.__instance is None:
            if struct.calcsize ("P") * 8 != 64:
                raise Exception ("You need 64-bit python to use this library")
            cls.__instance = cls ()
        return cls.__instance

    def __init__ (self):
        if platform.system () == 'Windows':
            dll_path = 'lib\\BoardController.dll'
            os.environ['Path'] = os.path.abspath (os.path.dirname (pkg_resources.resource_filename (__name__, os.path.join ('lib', 'BoardController.dll')))) + os.pathsep + os.environ['Path']
        elif platform.system () == 'Darwin':
            dll_path = 'lib/libBoardController.dylib'
        else:
            dll_path = 'lib/libBoardController.so'
        self.lib = ctypes.cdll.LoadLibrary (pkg_resources.resource_filename (__name__, dll_path))

        self.prepare_session = self.lib.prepare_session
        self.prepare_session.restype = ctypes.c_int
        self.prepare_session.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.start_stream = self.lib.start_stream
        self.start_stream.restype = ctypes.c_int
        self.start_stream.argtypes = [
            ctypes.c_int
        ]

        self.stop_stream = self.lib.stop_stream
        self.stop_stream.restype = ctypes.c_int
        self.stop_stream.argtypes = []

        self.get_current_board_data = self.lib.get_current_board_data
        self.get_current_board_data.restype = ctypes.c_int
        self.get_current_board_data.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_float),
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_int64)
        ]

        self.get_board_data = self.lib.get_board_data
        self.get_board_data.restype = ctypes.c_int
        self.get_board_data.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_float),
            ndpointer (ctypes.c_double)
        ]

        self.stop_stream = self.lib.stop_stream
        self.stop_stream.restype = ctypes.c_int
        self.stop_stream.argtypes = []

        self.release_session = self.lib.release_session
        self.release_session.restype = ctypes.c_int
        self.release_session.argtypes = []

        self.get_board_data_count = self.lib.get_board_data_count
        self.get_board_data_count.restype = ctypes.c_int
        self.get_board_data_count.argtypes = [
           ndpointer (ctypes.c_int64)
        ]

        self.set_log_level = self.lib.set_log_level
        self.set_log_level.restype = ctypes.c_int
        self.set_log_level.argtypes = [
           ctypes.c_int
        ]

class BoardShim (object):

    def __init__ (self, board_id, port_name):
        if port_name:
            if sys.version_info >= (3,0):
                self.port_name = port_name.encode ()
            else:
                self.port_name = port_name
        else:
            self.port_name = None
        self.board_id = board_id
        self.package_length = BoardInfoGetter.get_package_length (board_id)
        self.fs_hz = BoardInfoGetter.get_fs_hz (board_id)
        self.num_eeg_channels = BoardInfoGetter.get_num_eeg_channels (board_id)

    @classmethod
    def enable_board_logger (cls):
        res = BoardControllerDLL.get_instance ().set_log_level (2)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to enable logger', res)

    @classmethod
    def disable_board_logger (cls):
        res = BoardControllerDLL.get_instance ().set_log_level (6)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to disable logger', res)

    @classmethod
    def enable_dev_board_logger (cls):
        res = BoardControllerDLL.get_instance ().set_log_level (0)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to enable logger', res)

    def prepare_session (self):
        res = BoardControllerDLL.get_instance ().prepare_session (self.board_id, self.port_name)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to prepare streaming session', res)

    def start_stream (self, num_samples = 1800*250):
        res = BoardControllerDLL.get_instance ().start_stream (num_samples)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to start streaming session', res)

    def stop_stream (self):
        res = BoardControllerDLL.get_instance ().stop_stream ()
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to stop streaming session', res)

    def release_session (self):
        res = BoardControllerDLL.get_instance ().release_session ()
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to release streaming session', res)

    def get_current_board_data (self, num_samples):
        data_arr = numpy.zeros (int(num_samples  * self.package_length)).astype (numpy.float32)
        time_arr = numpy.zeros (num_samples).astype (numpy.float64)
        current_size = numpy.zeros (1).astype (numpy.int64)

        res = BoardControllerDLL.get_instance ().get_current_board_data (num_samples, data_arr, time_arr, current_size)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to get current data', res)

        if len (current_size) == 0:
            return None

        data_arr = data_arr[0:current_size[0] * self.package_length].reshape (current_size[0], self.package_length)
        time_arr = time_arr[0:current_size[0]]
        return numpy.column_stack ((data_arr, time_arr))

    def get_immediate_board_data (self):
        return self.get_current_board_data (1)

    def get_board_data_count (self):
        data_size = numpy.zeros (1).astype (numpy.int64)

        res = BoardControllerDLL.get_instance ().get_board_data_count (data_size)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to obtain buffer size', res)
        return data_size[0]

    def get_board_data (self):
        data_size = self.get_board_data_count ()
        time_arr = numpy.zeros (data_size).astype (numpy.float64)
        data_arr = numpy.zeros (data_size * self.package_length).astype (numpy.float32)

        res = BoardControllerDLL.get_instance ().get_board_data (data_size, data_arr, time_arr)
        if res != StreamExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to get board data', res)

        data_arr = data_arr.reshape (data_size, self.package_length)
        return numpy.column_stack ((data_arr, time_arr))
