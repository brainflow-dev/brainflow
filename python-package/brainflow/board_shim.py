import ctypes
import numpy
from numpy.ctypeslib import ndpointer
import pkg_resources
import enum
import os
import platform
import sys
import struct
import json

from brainflow.exit_codes import BrainflowExitCodes


class BoardIds (enum.Enum):
    """Enum to store all supported Board Ids"""

    STREAMING_BOARD = -2 #:
    SYNTHETIC_BOARD = -1 #:
    CYTON_BOARD = 0 #:
    GANGLION_BOARD = 1 #:
    CYTON_DAISY_BOARD = 2 #:
    NOVAXR_BOARD = 3 #:
    GANGLION_WIFI_BOARD = 4 #:
    CYTON_WIFI_BOARD = 5 #:
    CYTON_DAISY_WIFI_BOARD = 6 #:


class LogLevels (enum.Enum):
    """Enum to store all log levels supported by BrainFlow"""

    LEVEL_TRACE = 0 #:
    LEVEL_DEBUG = 1 #:
    LEVEL_INFO = 2 #:
    LEVEL_WARN = 3 #:
    LEVEL_ERROR = 4 #:
    LEVEL_CRITICAL = 5 #:
    LEVEL_OFF = 6 #:


class IpProtocolType (enum.Enum):
    """Enum to store Ip Protocol types"""

    NONE = 0 #:
    UDP = 1 #:
    TCP = 2 #:


class BrainFlowInputParams (object):
    """ inputs parameters for prepare_session method

    :param serial_port: serial port name is used for boards which reads data from serial port
    :type serial_port: str
    :param mac_address: mac address for example its used for bluetooth based boards
    :type mac_address: str
    :param ip_address: ip address is used for boards which reads data from socket connection
    :type ip_address: str
    :param ip_port: ip port for socket connection, for some boards where we know it in front you dont need this parameter(NovaXR for example)
    :type ip_port: int
    :param ip_protocol: ip protocol type from IpProtocolType enum
    :type ip_protocol: int
    :param other_info: other info
    :type other_info: str
    """
    def __init__ (self):
        self.serial_port = ''
        self.mac_address = ''
        self.ip_address = ''
        self.ip_port = 0
        self.ip_protocol = IpProtocolType.NONE.value
        self.other_info = ''

    def to_json (self):
        return json.dumps (self, default = lambda o: o.__dict__,
            sort_keys = True, indent = 4)

class BrainFlowError (Exception):
    """This exception is raised if non-zero exit code is returned from C code

    :param message: exception message
    :type message: str
    :param exit_code: exit code flow low level API
    :type exit_code: int
    """
    def __init__ (self, message, exit_code):
        detailed_message = '%s:%d %s' % (BrainflowExitCodes (exit_code).name, exit_code, message)
        super (BrainFlowError, self).__init__ (detailed_message)
        self.exit_code = exit_code


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

        self.is_prepared = self.lib.is_prepared
        self.is_prepared.restype = ctypes.c_int
        self.is_prepared.argtypes = [
            ndpointer (ctypes.c_int32),
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.start_stream = self.lib.start_stream
        self.start_stream.restype = ctypes.c_int
        self.start_stream.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p,
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
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_int32),
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.get_board_data = self.lib.get_board_data
        self.get_board_data.restype = ctypes.c_int
        self.get_board_data.argtypes = [
            ctypes.c_int,
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
            ndpointer (ctypes.c_int32),
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

        self.log_message = self.lib.log_message
        self.log_message.restype = ctypes.c_int
        self.log_message.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.config_board = self.lib.config_board
        self.config_board.restype = ctypes.c_int
        self.config_board.argtypes = [
            ctypes.c_char_p,
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.get_sampling_rate = self.lib.get_sampling_rate
        self.get_sampling_rate.restype = ctypes.c_int
        self.get_sampling_rate.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32)
        ]

        self.get_battery_channel = self.lib.get_battery_channel
        self.get_battery_channel.restype = ctypes.c_int
        self.get_battery_channel.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32)
        ]

        self.get_package_num_channel = self.lib.get_package_num_channel
        self.get_package_num_channel.restype = ctypes.c_int
        self.get_package_num_channel.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32)
        ]

        self.get_timestamp_channel = self.lib.get_timestamp_channel
        self.get_timestamp_channel.restype = ctypes.c_int
        self.get_timestamp_channel.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32)
        ]

        self.get_num_rows = self.lib.get_num_rows
        self.get_num_rows.restype = ctypes.c_int
        self.get_num_rows.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32)
        ]

        self.get_eeg_channels = self.lib.get_eeg_channels
        self.get_eeg_channels.restype = ctypes.c_int
        self.get_eeg_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_emg_channels = self.lib.get_emg_channels
        self.get_emg_channels.restype = ctypes.c_int
        self.get_emg_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_ecg_channels = self.lib.get_ecg_channels
        self.get_ecg_channels.restype = ctypes.c_int
        self.get_ecg_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_eog_channels = self.lib.get_eog_channels
        self.get_eog_channels.restype = ctypes.c_int
        self.get_eog_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_ppg_channels = self.lib.get_ppg_channels
        self.get_ppg_channels.restype = ctypes.c_int
        self.get_ppg_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_eda_channels = self.lib.get_eda_channels
        self.get_eda_channels.restype = ctypes.c_int
        self.get_eda_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_accel_channels = self.lib.get_accel_channels
        self.get_accel_channels.restype = ctypes.c_int
        self.get_accel_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_analog_channels = self.lib.get_analog_channels
        self.get_analog_channels.restype = ctypes.c_int
        self.get_analog_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_gyro_channels = self.lib.get_gyro_channels
        self.get_gyro_channels.restype = ctypes.c_int
        self.get_gyro_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_other_channels = self.lib.get_other_channels
        self.get_other_channels.restype = ctypes.c_int
        self.get_other_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]

        self.get_temperature_channels = self.lib.get_temperature_channels
        self.get_temperature_channels.restype = ctypes.c_int
        self.get_temperature_channels.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32)
        ]


class BoardShim (object):
    """BoardShim class is a primary interface to all boards

    :param board_id: Id of your board
    :type board_id: int
    :param input_params: board specific structure to pass required arguments
    :type input_params: BrainFlowInputParams
    """
    def __init__ (self, board_id, input_params):
        try:
            self.input_json = input_params.to_json ().encode ()
        except:
            self.input_json = input_params.to_json ()
        else:
            self.port_name = None
        self.board_id = board_id
        # we need it for streaming board
        if board_id == BoardIds.STREAMING_BOARD.value:
            try:
                self._master_board_id = int (input_params.other_info)
            except:
                raise BrainFlowError ('set master board id using params.other_info for STREAMING_BOARD',
                                    BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        else:
            self._master_board_id = self.board_id


    @classmethod
    def set_log_level (cls, log_level):
        """set BrainFlow log level, use it only if you want to write your own messages to BrainFlow logger,
        otherwise use enable_board_logger, enable_dev_board_logger or disable_board_logger

        :param log_level: log level, to specify it you should use values from LogLevels enum
        :type log_level: int
        """
        res = BoardControllerDLL.get_instance ().set_log_level (log_level)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to enable logger', res)

    @classmethod
    def enable_board_logger (cls):
        """enable BrainFlow Logger with level INFO, uses stderr for log messages by default"""
        cls.set_log_level (LogLevels.LEVEL_INFO.value)

    @classmethod
    def disable_board_logger (cls):
        """disable BrainFlow Logger"""
        cls.set_log_level (LogLevels.LEVEL_OFF.value)

    @classmethod
    def enable_dev_board_logger (cls):
        """enable BrainFlow Logger with level TRACE, uses stderr for log messages by default"""
        cls.set_log_level (LogLevels.LEVEL_TRACE.value)

    @classmethod
    def log_message (cls, log_level, message):
        """write your own log message to BrainFlow logger, use it if you wanna have single logger for your own code and BrainFlow's code

        :param log_level: log level
        :type log_file: int
        :param message: message
        :type message: str
        """
        try:
            msg = message.encode ()
        except:
            msg = message
        res = BoardControllerDLL.get_instance ().log_message (log_level, msg)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to write log message', res)

    @classmethod
    def set_log_file (cls, log_file):
        """redirect logger from stderr to file, can be called any time

        :param log_file: log file name
        :type log_file: str
        """
        try:
            file = log_file.encode ()
        except:
            file = log_file
        res = BoardControllerDLL.get_instance ().set_log_file (file)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to redirect logs to a file', res)

    @classmethod
    def get_sampling_rate (cls, board_id):
        """get sampling rate for a board

        :param board_id: Board Id
        :type board_id: int
        :return: sampling rate for this board id
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        sampling_rate = numpy.zeros (1).astype (numpy.int32)
        res = BoardControllerDLL.get_instance ().get_sampling_rate (board_id, sampling_rate)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        return int (sampling_rate[0])

    @classmethod
    def get_package_num_channel (cls, board_id):
        """get package num channel for a board

        :param board_id: Board Id
        :type board_id: int
        :return: number of package num channel
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        package_num_channel = numpy.zeros (1).astype (numpy.int32)
        res = BoardControllerDLL.get_instance ().get_package_num_channel (board_id, package_num_channel)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        return int (package_num_channel[0])

    @classmethod
    def get_battery_channel (cls, board_id):
        """get battery channel for a board

        :param board_id: Board Id
        :type board_id: int
        :return: number of batter channel
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        battery_channel = numpy.zeros (1).astype (numpy.int32)
        res = BoardControllerDLL.get_instance ().get_battery_channel (board_id, battery_channel)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        return int (battery_channel[0])

    @classmethod
    def get_num_rows (cls, board_id):
        """get number of rows in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: number of rows in returned numpy array
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_rows = numpy.zeros (1).astype (numpy.int32)
        res = BoardControllerDLL.get_instance ().get_num_rows (board_id, num_rows)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        return int (num_rows[0])

    @classmethod
    def get_timestamp_channel (cls, board_id):
        """get timestamp channel in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: number of timestamp channel in returned numpy array
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        timestamp_channel = numpy.zeros (1).astype (numpy.int32)
        res = BoardControllerDLL.get_instance ().get_timestamp_channel (board_id, timestamp_channel)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        return int (timestamp_channel[0])

    @classmethod
    def get_eeg_channels (cls, board_id):
        """get list of eeg channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of eeg channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        eeg_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_eeg_channels (board_id, eeg_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = eeg_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_emg_channels (cls, board_id):
        """get list of emg channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of eeg channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        emg_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_emg_channels (board_id, emg_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = emg_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_ecg_channels (cls, board_id):
        """get list of ecg channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of ecg channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        ecg_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_ecg_channels (board_id, ecg_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = ecg_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_eog_channels (cls, board_id):
        """get list of eog channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of eog channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        eog_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_eog_channels (board_id, eog_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = eog_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_eda_channels (cls, board_id):
        """get list of eda channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of eda channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        eda_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_eda_channels (board_id, eda_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = eda_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_ppg_channels (cls, board_id):
        """get list of ppg channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of ppg channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        ppg_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_ppg_channels (board_id, ppg_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = ppg_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_accel_channels (cls, board_id):
        """get list of accel channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of accel channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        accel_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_accel_channels (board_id, accel_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = accel_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_analog_channels (cls, board_id):
        """get list of analog channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of analog channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        analog_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_analog_channels (board_id, analog_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = analog_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_gyro_channels (cls, board_id):
        """get list of gyro channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of gyro channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        gyro_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_gyro_channels (board_id, gyro_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = gyro_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_other_channels (cls, board_id):
        """get list of other channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of other channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        other_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_other_channels (board_id, other_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = other_channels.tolist () [0:num_channels[0]]
        return result

    @classmethod
    def get_temperature_channels (cls, board_id):
        """get list of temperature channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :return: list of temperature channels in returned numpy array
        :rtype: list
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """
        num_channels = numpy.zeros (1).astype (numpy.int32)
        temperature_channels = numpy.zeros (512).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_temperature_channels (board_id, temperature_channels, num_channels)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to request info about this board', res)
        result = temperature_channels.tolist () [0:num_channels[0]]
        return result

    def prepare_session (self):
        """prepare streaming sesssion, init resources, you need to call it before any other BoardShim object methods"""

        res = BoardControllerDLL.get_instance ().prepare_session (self.board_id, self.input_json)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to prepare streaming session', res)

    def start_stream (self, num_samples = 1800 * 250, streamer_params = None):
        """Start streaming data, this methods stores data in ringbuffer

        :param num_samples: size of ring buffer to keep data
        :type num_samples: int
        :param streamer_params parameter to stream data from brainflow, supported vals: "file://%file_name%:w", "file://%file_name%:a", "streaming_board://%multicast_group_ip%:%port%". Range for multicast addresses is from "224.0.0.0" to "239.255.255.255"
        :type streamer_params: str
        """

        if streamer_params is None:
            streamer = None
        else:
            try:
                streamer = streamer_params.encode ()
            except:
                streamer = streamer_params

        res = BoardControllerDLL.get_instance ().start_stream (num_samples, streamer, self.board_id, self.input_json)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to start streaming session', res)

    def stop_stream (self):
        """Stop streaming data"""

        res = BoardControllerDLL.get_instance ().stop_stream (self.board_id, self.input_json)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to stop streaming session', res)

    def release_session (self):
        """release all resources"""

        res = BoardControllerDLL.get_instance ().release_session (self.board_id, self.input_json)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to release streaming session', res)

    def get_current_board_data (self, num_samples):
        """Get specified amount of data or less if there is not enough data, doesnt remove data from ringbuffer

        :param num_samples: max number of samples
        :type num_samples: int
        :return: latest data from a board
        :rtype: numpy 2d array
        """
        package_length = BoardShim.get_num_rows (self._master_board_id)
        data_arr = numpy.zeros (int(num_samples  * package_length)).astype (numpy.float64)
        current_size = numpy.zeros (1).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_current_board_data (num_samples, data_arr, current_size, self.board_id, self.input_json)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to get current data', res)

        if len (current_size) == 0:
            return None

        data_arr = data_arr[0:current_size[0] * package_length].reshape (package_length, current_size[0])
        return data_arr

    def get_board_data_count (self):
        """Get num of elements in ringbuffer

        :return: number of elements in ring buffer
        :rtype: int
        """
        data_size = numpy.zeros (1).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().get_board_data_count (data_size, self.board_id, self.input_json)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to obtain buffer size', res)
        return data_size[0]

    def is_prepared (self):
        """Check if session is ready or not

        :return: session status
        :rtype: bool
        """
        prepared = numpy.zeros (1).astype (numpy.int32)

        res = BoardControllerDLL.get_instance ().is_prepared (prepared, self.board_id, self.input_json)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to check session status', res)
        return bool(prepared[0])

    def get_board_data (self):
        """Get all board data and remove them from ringbuffer

        :return: all data from a board
        :rtype: numpy 2d array
        """
        data_size = self.get_board_data_count ()
        package_length = BoardShim.get_num_rows (self._master_board_id)
        data_arr = numpy.zeros (data_size * package_length).astype (numpy.float64)

        res = BoardControllerDLL.get_instance ().get_board_data (data_size, data_arr, self.board_id, self.input_json)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to get board data', res)

        return data_arr.reshape (package_length, data_size)

    def config_board (self, config):
        """Use this method carefully and only if you understand what you are doing, do NOT use it to start or stop streaming

        :param config: string to send to a board
        :type config: str
        """
        try:
            config_string = config.encode ()
        except:
            config_string = config

        res = BoardControllerDLL.get_instance ().config_board (config_string, self.board_id, self.input_json)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to config board', res)
