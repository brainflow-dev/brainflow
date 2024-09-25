import ctypes
import enum
import json
import os
import platform
import struct
from typing import List

import numpy
import pkg_resources
from brainflow.exit_codes import BrainFlowExitCodes, BrainFlowError
from brainflow.utils import LogLevels
from numpy.ctypeslib import ndpointer


class BoardIds(enum.IntEnum):
    """Enum to store all supported Board Ids"""

    NO_BOARD = -100
    PLAYBACK_FILE_BOARD = -3  #:
    STREAMING_BOARD = -2  #:
    SYNTHETIC_BOARD = -1  #:
    CYTON_BOARD = 0  #:
    GANGLION_BOARD = 1  #:
    CYTON_DAISY_BOARD = 2  #:
    GALEA_BOARD = 3  #:
    GANGLION_WIFI_BOARD = 4  #:
    CYTON_WIFI_BOARD = 5  #:
    CYTON_DAISY_WIFI_BOARD = 6  #:
    BRAINBIT_BOARD = 7  #:
    UNICORN_BOARD = 8  #:
    CALLIBRI_EEG_BOARD = 9  #:
    CALLIBRI_EMG_BOARD = 10  #:
    CALLIBRI_ECG_BOARD = 11  #:
    NOTION_1_BOARD = 13  #:
    NOTION_2_BOARD = 14  #:
    GFORCE_PRO_BOARD = 16  #:
    FREEEEG32_BOARD = 17  #:
    BRAINBIT_BLED_BOARD = 18  #:
    GFORCE_DUAL_BOARD = 19  #:
    GALEA_SERIAL_BOARD = 20  #:
    MUSE_S_BLED_BOARD = 21  #:
    MUSE_2_BLED_BOARD = 22  #:
    CROWN_BOARD = 23  #:
    ANT_NEURO_EE_410_BOARD = 24  #:
    ANT_NEURO_EE_411_BOARD = 25  #:
    ANT_NEURO_EE_430_BOARD = 26  #:
    ANT_NEURO_EE_211_BOARD = 27  #:
    ANT_NEURO_EE_212_BOARD = 28  #:
    ANT_NEURO_EE_213_BOARD = 29  #:
    ANT_NEURO_EE_214_BOARD = 30  #:
    ANT_NEURO_EE_215_BOARD = 31  #:
    ANT_NEURO_EE_221_BOARD = 32  #:
    ANT_NEURO_EE_222_BOARD = 33  #:
    ANT_NEURO_EE_223_BOARD = 34  #:
    ANT_NEURO_EE_224_BOARD = 35  #:
    ANT_NEURO_EE_225_BOARD = 36  #:
    ENOPHONE_BOARD = 37  #:
    MUSE_2_BOARD = 38  #:
    MUSE_S_BOARD = 39  #:
    BRAINALIVE_BOARD = 40  #:
    MUSE_2016_BOARD = 41  #:
    MUSE_2016_BLED_BOARD = 42  #:
    EXPLORE_4_CHAN_BOARD = 44  #:
    EXPLORE_8_CHAN_BOARD = 45  #:
    GANGLION_NATIVE_BOARD = 46  #:
    EMOTIBIT_BOARD = 47  #:
    GALEA_BOARD_V4 = 48  #:
    GALEA_SERIAL_BOARD_V4 = 49  #:
    NTL_WIFI_BOARD = 50  #:
    ANT_NEURO_EE_511_BOARD = 51  #:
    FREEEEG128_BOARD = 52  #:
    AAVAA_V3_BOARD = 53 #:
    EXPLORE_PLUS_8_CHAN_BOARD = 54 #:
    EXPLORE_PLUS_32_CHAN_BOARD = 55 #:
    PIEEG_BOARD = 56  #:
    NEUROPAWN_KNIGHT_BOARD = 57 #:


class IpProtocolTypes(enum.IntEnum):
    """Enum to store Ip Protocol types"""

    NO_IP_PROTOCOL = 0  #:
    UDP = 1  #:
    TCP = 2  #:


class BrainFlowPresets(enum.IntEnum):
    """Enum to store presets"""

    DEFAULT_PRESET = 0  #:
    AUXILIARY_PRESET = 1  #:
    ANCILLARY_PRESET = 2  #:


class BrainFlowInputParams(object):
    """ inputs parameters for prepare_session method

    :param serial_port: serial port name is used for boards which reads data from serial port
    :type serial_port: str
    :param mac_address: mac address for example its used for bluetooth based boards
    :type mac_address: str
    :param ip_address: ip address is used for boards which reads data from socket connection
    :type ip_address: str
    :param ip_address_aux: ip address is used for boards which reads data from socket connection
    :type ip_address_aux: str
    :param ip_address_anc: ip address is used for boards which reads data from socket connection
    :type ip_address_anc: str
    :param ip_port: ip port for socket connection, for some boards where we know it in front you dont need this parameter
    :type ip_port: int
    :param ip_port_aux: ip port for socket connection, for some boards where we know it in front you dont need this parameter
    :type ip_port_aux: int
    :param ip_port_anc: ip port for socket connection, for some boards where we know it in front you dont need this parameter
    :type ip_port_anc: int
    :param ip_protocol: ip protocol type from IpProtocolTypes enum
    :type ip_protocol: int
    :param other_info: other info
    :type other_info: str
    :param serial_number: serial number
    :type serial_number: str
    :param file: file
    :type file: str
    :param file_aux: file
    :type file_aux: str
    :param file_anc: file
    :type file_anc: str
    """

    def __init__(self) -> None:
        self.serial_port = ''
        self.mac_address = ''
        self.ip_address = ''
        self.ip_address_aux = ''
        self.ip_address_anc = ''
        self.ip_port = 0
        self.ip_port_aux = 0
        self.ip_port_anc = 0
        self.ip_protocol = IpProtocolTypes.NO_IP_PROTOCOL.value
        self.other_info = ''
        self.timeout = 0
        self.serial_number = ''
        self.file = ''
        self.file_aux = ''
        self.file_anc = ''
        self.master_board = BoardIds.NO_BOARD.value

    def to_json(self) -> None:
        return json.dumps(self, default=lambda o: o.__dict__,
                          sort_keys=True, indent=4)


class BoardControllerDLL(object):
    __instance = None

    @classmethod
    def get_instance(cls):
        if cls.__instance is None:
            cls.__instance = cls()
        return cls.__instance

    def __init__(self):
        if platform.system() == 'Windows':
            if struct.calcsize("P") * 8 == 64:
                dll_path = 'lib\\BoardController.dll'
            else:
                dll_path = 'lib\\BoardController32.dll'
        elif platform.system() == 'Darwin':
            dll_path = 'lib/libBoardController.dylib'
        else:
            dll_path = 'lib/libBoardController.so'
        full_path = pkg_resources.resource_filename(__name__, dll_path)
        if os.path.isfile(full_path):
            dir_path = os.path.abspath(os.path.dirname(full_path))
            # for python we load dll by direct path but this dll may depend on other dlls and they will not be found!
            # to solve it we can load all of them before loading the main one or change PATH\LD_LIBRARY_PATH env var.
            # env variable looks better, since it can be done only once for all dependencies
            # for python 3.8 PATH env var doesnt work anymore
            try:
                os.add_dll_directory(dir_path)
            except:
                pass
            if platform.system() == 'Windows':
                os.environ['PATH'] = dir_path + os.pathsep + os.environ.get('PATH', '')
            else:
                os.environ['LD_LIBRARY_PATH'] = dir_path + os.pathsep + os.environ.get('LD_LIBRARY_PATH', '')
            self.lib = ctypes.cdll.LoadLibrary(full_path)
        else:
            raise FileNotFoundError(
                'Dynamic library %s is missed, did you forget to compile brainflow before installation of python package?' % full_path)

        self.prepare_session = self.lib.prepare_session
        self.prepare_session.restype = ctypes.c_int
        self.prepare_session.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.is_prepared = self.lib.is_prepared
        self.is_prepared.restype = ctypes.c_int
        self.is_prepared.argtypes = [
            ndpointer(ctypes.c_int32),
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

        self.add_streamer = self.lib.add_streamer
        self.add_streamer.restype = ctypes.c_int
        self.add_streamer.argtypes = [
            ctypes.c_char_p,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.delete_streamer = self.lib.delete_streamer
        self.delete_streamer.restype = ctypes.c_int
        self.delete_streamer.argtypes = [
            ctypes.c_char_p,
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
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_int32),
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.get_board_data = self.lib.get_board_data
        self.get_board_data.restype = ctypes.c_int
        self.get_board_data.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.release_session = self.lib.release_session
        self.release_session.restype = ctypes.c_int
        self.release_session.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.release_all_sessions = self.lib.release_all_sessions
        self.release_all_sessions.restype = ctypes.c_int
        self.release_all_sessions.argtypes = []

        self.insert_marker = self.lib.insert_marker
        self.insert_marker.restype = ctypes.c_int
        self.insert_marker.argtypes = [
            ctypes.c_double,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.get_board_data_count = self.lib.get_board_data_count
        self.get_board_data_count.restype = ctypes.c_int
        self.get_board_data_count.argtypes = [
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.set_log_level_board_controller = self.lib.set_log_level_board_controller
        self.set_log_level_board_controller.restype = ctypes.c_int
        self.set_log_level_board_controller.argtypes = [
            ctypes.c_int
        ]

        self.set_log_file_board_controller = self.lib.set_log_file_board_controller
        self.set_log_file_board_controller.restype = ctypes.c_int
        self.set_log_file_board_controller.argtypes = [
            ctypes.c_char_p
        ]

        self.log_message_board_controller = self.lib.log_message_board_controller
        self.log_message_board_controller.restype = ctypes.c_int
        self.log_message_board_controller.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.config_board = self.lib.config_board
        self.config_board.restype = ctypes.c_int
        self.config_board.argtypes = [
            ctypes.c_char_p,
            ndpointer(ctypes.c_ubyte),
            ndpointer(ctypes.c_int32),
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.config_board_with_bytes = self.lib.config_board_with_bytes
        self.config_board_with_bytes.restype = ctypes.c_int
        self.config_board_with_bytes.argtypes = [
            ndpointer(ctypes.c_ubyte),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.get_sampling_rate = self.lib.get_sampling_rate
        self.get_sampling_rate.restype = ctypes.c_int
        self.get_sampling_rate.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32)
        ]

        self.get_battery_channel = self.lib.get_battery_channel
        self.get_battery_channel.restype = ctypes.c_int
        self.get_battery_channel.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32)
        ]

        self.get_package_num_channel = self.lib.get_package_num_channel
        self.get_package_num_channel.restype = ctypes.c_int
        self.get_package_num_channel.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32)
        ]

        self.get_timestamp_channel = self.lib.get_timestamp_channel
        self.get_timestamp_channel.restype = ctypes.c_int
        self.get_timestamp_channel.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32)
        ]

        self.get_marker_channel = self.lib.get_marker_channel
        self.get_marker_channel.restype = ctypes.c_int
        self.get_marker_channel.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32)
        ]

        self.get_num_rows = self.lib.get_num_rows
        self.get_num_rows.restype = ctypes.c_int
        self.get_num_rows.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32)
        ]

        self.get_eeg_names = self.lib.get_eeg_names
        self.get_eeg_names.restype = ctypes.c_int
        self.get_eeg_names.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_ubyte),
            ndpointer(ctypes.c_int32)
        ]

        self.get_board_presets = self.lib.get_board_presets
        self.get_board_presets.restype = ctypes.c_int
        self.get_board_presets.argtypes = [
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_version_board_controller = self.lib.get_version_board_controller
        self.get_version_board_controller.restype = ctypes.c_int
        self.get_version_board_controller.argtypes = [
            ndpointer(ctypes.c_ubyte),
            ndpointer(ctypes.c_int32),
            ctypes.c_int
        ]

        self.get_board_descr = self.lib.get_board_descr
        self.get_board_descr.restype = ctypes.c_int
        self.get_board_descr.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_ubyte),
            ndpointer(ctypes.c_int32)
        ]

        self.get_device_name = self.lib.get_device_name
        self.get_device_name.restype = ctypes.c_int
        self.get_device_name.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_ubyte),
            ndpointer(ctypes.c_int32)
        ]

        self.get_eeg_channels = self.lib.get_eeg_channels
        self.get_eeg_channels.restype = ctypes.c_int
        self.get_eeg_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_exg_channels = self.lib.get_exg_channels
        self.get_exg_channels.restype = ctypes.c_int
        self.get_exg_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_emg_channels = self.lib.get_emg_channels
        self.get_emg_channels.restype = ctypes.c_int
        self.get_emg_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_ecg_channels = self.lib.get_ecg_channels
        self.get_ecg_channels.restype = ctypes.c_int
        self.get_ecg_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_eog_channels = self.lib.get_eog_channels
        self.get_eog_channels.restype = ctypes.c_int
        self.get_eog_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_ppg_channels = self.lib.get_ppg_channels
        self.get_ppg_channels.restype = ctypes.c_int
        self.get_ppg_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_eda_channels = self.lib.get_eda_channels
        self.get_eda_channels.restype = ctypes.c_int
        self.get_eda_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_accel_channels = self.lib.get_accel_channels
        self.get_accel_channels.restype = ctypes.c_int
        self.get_accel_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_rotation_channels = self.lib.get_rotation_channels
        self.get_rotation_channels.restype = ctypes.c_int
        self.get_rotation_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_analog_channels = self.lib.get_analog_channels
        self.get_analog_channels.restype = ctypes.c_int
        self.get_analog_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_gyro_channels = self.lib.get_gyro_channels
        self.get_gyro_channels.restype = ctypes.c_int
        self.get_gyro_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_other_channels = self.lib.get_other_channels
        self.get_other_channels.restype = ctypes.c_int
        self.get_other_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_temperature_channels = self.lib.get_temperature_channels
        self.get_temperature_channels.restype = ctypes.c_int
        self.get_temperature_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_resistance_channels = self.lib.get_resistance_channels
        self.get_resistance_channels.restype = ctypes.c_int
        self.get_resistance_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]

        self.get_magnetometer_channels = self.lib.get_magnetometer_channels
        self.get_magnetometer_channels.restype = ctypes.c_int
        self.get_magnetometer_channels.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32)
        ]


class BoardShim(object):
    """BoardShim class is a primary interface to all boards

    :param board_id: Id of your board
    :type board_id: int
    :param input_params: board specific structure to pass required arguments
    :type input_params: BrainFlowInputParams
    """

    def __init__(self, board_id: int, input_params: BrainFlowInputParams) -> None:
        try:
            self.input_json = input_params.to_json().encode()
        except BaseException:
            self.input_json = input_params.to_json()
        self.board_id = board_id
        # we need it for streaming board
        if board_id == BoardIds.STREAMING_BOARD.value or board_id == BoardIds.PLAYBACK_FILE_BOARD.value:
            if input_params.master_board != BoardIds.NO_BOARD:
                self._master_board_id = input_params.master_board
            else:
                raise BrainFlowError('you need set master board id in BrainFlowInputParams',
                                     BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        else:
            self._master_board_id = self.board_id

    @classmethod
    def set_log_level(cls, log_level: int) -> None:
        """set BrainFlow log level, use it only if you want to write your own messages to BrainFlow logger,
        otherwise use enable_board_logger, enable_dev_board_logger or disable_board_logger

        :param log_level: log level, to specify it you should use values from LogLevels enum
        :type log_level: int
        """
        res = BoardControllerDLL.get_instance().set_log_level_board_controller(log_level)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to enable logger', res)

    @classmethod
    def enable_board_logger(cls) -> None:
        """enable BrainFlow Logger with level INFO, uses stderr for log messages by default"""
        cls.set_log_level(LogLevels.LEVEL_INFO.value)

    @classmethod
    def disable_board_logger(cls) -> None:
        """disable BrainFlow Logger"""
        cls.set_log_level(LogLevels.LEVEL_OFF.value)

    @classmethod
    def enable_dev_board_logger(cls) -> None:
        """enable BrainFlow Logger with level TRACE, uses stderr for log messages by default"""
        cls.set_log_level(LogLevels.LEVEL_TRACE.value)

    @classmethod
    def log_message(cls, log_level: int, message: str) -> None:
        """write your own log message to BrainFlow logger, use it if you wanna have single logger for your own code and BrainFlow's code

        :param log_level: log level
        :type log_file: int
        :param message: message
        :type message: str
        """
        try:
            msg = message.encode()
        except BaseException:
            msg = message
        res = BoardControllerDLL.get_instance().log_message_board_controller(log_level, msg)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to write log message', res)

    @classmethod
    def set_log_file(cls, log_file: str) -> None:
        """redirect logger from stderr to file, can be called any time

        :param log_file: log file name
        :type log_file: str
        """
        try:
            file = log_file.encode()
        except BaseException:
            file = log_file
        res = BoardControllerDLL.get_instance().set_log_file_board_controller(file)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to redirect logs to a file', res)

    @classmethod
    def get_sampling_rate(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> int:
        """get sampling rate for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: sampling rate for this board id
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        sampling_rate = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_sampling_rate(board_id, preset, sampling_rate)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        return int(sampling_rate[0])

    @classmethod
    def get_package_num_channel(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> int:
        """get package num channel for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: number of package num channel
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        package_num_channel = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_package_num_channel(board_id, preset, package_num_channel)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        return int(package_num_channel[0])

    @classmethod
    def get_battery_channel(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> int:
        """get battery channel for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: number of batter channel
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        battery_channel = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_battery_channel(board_id, preset, battery_channel)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        return int(battery_channel[0])

    @classmethod
    def get_num_rows(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> int:
        """get number of rows in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: number of rows in returned numpy array
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_rows = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_num_rows(board_id, preset, num_rows)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        return int(num_rows[0])

    @classmethod
    def get_timestamp_channel(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> int:
        """get timestamp channel in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: number of timestamp channel in returned numpy array
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        timestamp_channel = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_timestamp_channel(board_id, preset, timestamp_channel)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        return int(timestamp_channel[0])

    @classmethod
    def get_marker_channel(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> int:
        """get marker channel in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: number of marker channel in returned numpy array
        :rtype: int
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        marker_channel = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_marker_channel(board_id, preset, marker_channel)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        return int(marker_channel[0])

    @classmethod
    def get_eeg_names(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[str]:
        """get names of EEG channels in 10-20 system if their location is fixed

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: EEG channels names
        :rtype: List[str]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        string = numpy.zeros(4096).astype(numpy.ubyte)
        string_len = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_eeg_names(board_id, preset, string, string_len)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        return string.tobytes().decode('utf-8')[0:string_len[0]].split(',')

    @classmethod
    def get_board_presets(cls, board_id: int) -> List[str]:
        """get available presets for this board id

        :param board_id: Board Id
        :type board_id: int
        :return: presets for this board id
        :rtype: List[str]
        :raises BrainFlowError: In case of internal error or invalid args
        """

        num_presets = numpy.zeros(1).astype(numpy.int32)
        presets = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_board_presets(board_id, presets, num_presets)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = presets.tolist()[0:num_presets[0]]
        return result

    @classmethod
    def get_version(cls) -> str:
        """get version of brainflow libraries

        :return: version
        :rtype: str
        :raises BrainFlowError: In case of internal error or invalid args
        """
        string = numpy.zeros(64).astype(numpy.ubyte)
        string_len = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_version_board_controller(string, string_len, 64)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info', res)
        return string.tobytes().decode('utf-8')[0:string_len[0]]

    @classmethod
    def get_board_descr(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET):
        """get board description as json

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: info about board
        :rtype: json
        :raises BrainFlowError: If there is no such board id exit code is UNSUPPORTED_BOARD_ERROR
        """

        string = numpy.zeros(16000).astype(numpy.ubyte)
        string_len = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_board_descr(board_id, preset, string, string_len)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        return json.loads(string.tobytes().decode('utf-8')[0:string_len[0]])

    @classmethod
    def get_device_name(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> str:
        """get device name

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: Device Name
        :rtype: str
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        string = numpy.zeros(4096).astype(numpy.ubyte)
        string_len = numpy.zeros(1).astype(numpy.int32)
        res = BoardControllerDLL.get_instance().get_device_name(board_id, preset, string, string_len)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        return string.tobytes().decode('utf-8')[0:string_len[0]]

    @classmethod
    def get_eeg_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of eeg channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of eeg channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        eeg_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_eeg_channels(board_id, preset, eeg_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = eeg_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_exg_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of exg channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of eeg channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        exg_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_exg_channels(board_id, preset, exg_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = exg_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_emg_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of emg channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of eeg channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        emg_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_emg_channels(board_id, preset, emg_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = emg_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_ecg_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of ecg channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of ecg channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        ecg_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_ecg_channels(board_id, preset, ecg_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = ecg_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_eog_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of eog channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of eog channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        eog_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_eog_channels(board_id, preset, eog_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = eog_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_eda_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of eda channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of eda channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        eda_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_eda_channels(board_id, preset, eda_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = eda_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_ppg_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of ppg channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of ppg channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        ppg_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_ppg_channels(board_id, preset, ppg_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = ppg_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_accel_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of accel channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of accel channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        accel_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_accel_channels(board_id, preset, accel_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = accel_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_rotation_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of rotation channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of rotation channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        rotation_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_rotation_channels(board_id, preset, rotation_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = rotation_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_analog_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of analog channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of analog channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        analog_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_analog_channels(board_id, preset, analog_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = analog_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_gyro_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of gyro channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of gyro channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        gyro_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_gyro_channels(board_id, preset, gyro_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = gyro_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_other_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of other channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of other channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        other_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_other_channels(board_id, preset, other_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = other_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_temperature_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of temperature channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of temperature channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        temperature_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_temperature_channels(board_id, preset, temperature_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = temperature_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_resistance_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of resistance channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of resistance channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        resistance_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_resistance_channels(board_id, preset, resistance_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = resistance_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def get_magnetometer_channels(cls, board_id: int, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> List[int]:
        """get list of magnetometer channels in resulting data table for a board

        :param board_id: Board Id
        :type board_id: int
        :param preset: preset
        :type preset: int
        :return: list of magnetometer channels in returned numpy array
        :rtype: List[int]
        :raises BrainFlowError: If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR
        """

        num_channels = numpy.zeros(1).astype(numpy.int32)
        magnetometer_channels = numpy.zeros(512).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_magnetometer_channels(board_id, preset, magnetometer_channels, num_channels)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info about this board', res)
        result = magnetometer_channels.tolist()[0:num_channels[0]]
        return result

    @classmethod
    def release_all_sessions(cls) -> None:
        """release all prepared sessions"""

        res = BoardControllerDLL.get_instance().release_all_sessions()
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to release sessions', res)

    def prepare_session(self) -> None:
        """prepare streaming sesssion, init resources, you need to call it before any other BoardShim object methods"""

        res = BoardControllerDLL.get_instance().prepare_session(self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to prepare streaming session', res)

    def add_streamer(self, streamer_params: str, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> None:
        """Add streamer

        :param preset: preset
        :type preset: int
        :param streamer_params parameter to stream data from brainflow, supported vals: "file://%file_name%:w", "file://%file_name%:a", "streaming_board://%multicast_group_ip%:%port%". Range for multicast addresses is from "224.0.0.0" to "239.255.255.255"
        :type streamer_params: str
        """

        if streamer_params is None:
            streamer = None
        else:
            try:
                streamer = streamer_params.encode()
            except BaseException:
                streamer = streamer_params

        res = BoardControllerDLL.get_instance().add_streamer(streamer, preset, self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to add streamer', res)

    def delete_streamer(self, streamer_params: str, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> None:
        """Delete streamer

        :param preset: preset
        :type preset: int
        :param streamer_params parameter to stream data from brainflow, supported vals: "file://%file_name%:w", "file://%file_name%:a", "streaming_board://%multicast_group_ip%:%port%". Range for multicast addresses is from "224.0.0.0" to "239.255.255.255"
        :type streamer_params: str
        """

        if streamer_params is None:
            streamer = None
        else:
            try:
                streamer = streamer_params.encode()
            except BaseException:
                streamer = streamer_params

        res = BoardControllerDLL.get_instance().delete_streamer(streamer, preset, self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to delete streamer', res)

    def start_stream(self, num_samples: int = 1800 * 250, streamer_params: str = None) -> None:
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
                streamer = streamer_params.encode()
            except BaseException:
                streamer = streamer_params

        res = BoardControllerDLL.get_instance().start_stream(num_samples, streamer, self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to start streaming session', res)

    def stop_stream(self) -> None:
        """Stop streaming data"""

        res = BoardControllerDLL.get_instance().stop_stream(self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to stop streaming session', res)

    def release_session(self) -> None:
        """release all resources"""

        res = BoardControllerDLL.get_instance().release_session(self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to release streaming session', res)

    def get_current_board_data(self, num_samples: int, preset: int = BrainFlowPresets.DEFAULT_PRESET):
        """Get specified amount of data or less if there is not enough data, doesnt remove data from ringbuffer

        :param num_samples: max number of samples
        :type num_samples: int
        :param preset: preset
        :type preset: int
        :return: latest data from a board
        :rtype: NDArray[Shape["*, *"], Float64]
        """

        package_length = BoardShim.get_num_rows(self._master_board_id, preset)
        data_arr = numpy.zeros(int(num_samples * package_length)).astype(numpy.float64)
        current_size = numpy.zeros(1).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_current_board_data(num_samples, preset, data_arr, current_size,
                                                                       self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to get current data', res)

        if len(current_size) == 0:
            return None

        data_arr = data_arr[0:current_size[0] * package_length].reshape(package_length, current_size[0])
        return data_arr

    def get_board_data_count(self, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> int:
        """Get num of elements in ringbuffer

        :param preset: preset
        :type preset: int
        :return: number of elements in ring buffer
        :rtype: int
        """

        data_size = numpy.zeros(1).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().get_board_data_count(preset, data_size, self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to obtain buffer size', res)
        return data_size[0]

    def get_board_id(self) -> int:
        """Get's the actual board id, can be different than provided

        :return: board id
        :rtype: int
        """

        return self._master_board_id

    def insert_marker(self, value: float, preset: int = BrainFlowPresets.DEFAULT_PRESET) -> None:
        """Insert Marker to Data Stream

        :param value: value to insert
        :type value: float
        :param preset: preset
        :type preset: int
        :return: board id
        :rtype: int
        """

        res = BoardControllerDLL.get_instance().insert_marker(value, preset, self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to insert marker', res)

    def is_prepared(self) -> bool:
        """Check if session is ready or not

        :return: session status
        :rtype: bool
        """
        prepared = numpy.zeros(1).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().is_prepared(prepared, self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to check session status', res)
        return bool(prepared[0])

    def get_board_data(self, num_samples=None, preset: int = BrainFlowPresets.DEFAULT_PRESET):
        """Get board data and remove data from ringbuffer

        :param num_samples: number of packages to get
        :type num_samples: int
        :param preset: preset
        :type preset: int
        :return: all data from a board if num_samples is None, num_samples packages or less if not None
        :rtype: NDArray[Shape["*, *"], Float64]
        """

        data_size = self.get_board_data_count(preset)
        if num_samples is not None:
            if num_samples < 1:
                raise BrainFlowError('invalid num_samples', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
            else:
                data_size = min(data_size, num_samples)
        package_length = BoardShim.get_num_rows(self._master_board_id, preset)
        data_arr = numpy.zeros(data_size * package_length).astype(numpy.float64)

        res = BoardControllerDLL.get_instance().get_board_data(data_size, preset, data_arr, self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to get board data', res)

        return data_arr.reshape(package_length, data_size)

    def config_board(self, config) -> str:
        """Use this method carefully and only if you understand what you are doing, do NOT use it to start or stop streaming

        :param config: string to send to a board
        :type config: str
        :return: response string if any
        :rtype: str
        """
        try:
            config_string = config.encode()
        except BaseException:
            config_string = config
        string = numpy.zeros(4096).astype(numpy.ubyte)
        string_len = numpy.zeros(1).astype(numpy.int32)

        res = BoardControllerDLL.get_instance().config_board(config_string, string, string_len, self.board_id,
                                                             self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to config board', res)
        return string.tobytes().decode('utf-8')[0:string_len[0]]

    def config_board_with_bytes(self, bytes_to_send) -> None:
        """Use this method carefully and only if you understand what you are doing
    
        :param bytes_to_send: bytes to send
        :type config: ndarray astype(numpy.ubyte)
        """
        res = BoardControllerDLL.get_instance().config_board_with_bytes(bytes_to_send, len(bytes_to_send), self.board_id, self.input_json)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to config board', res)
