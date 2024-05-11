import ctypes
import enum
import json
import os
import platform
import struct
from typing import List

import numpy
import pkg_resources
from brainflow.board_shim import BrainFlowError, LogLevels
from brainflow.exit_codes import BrainFlowExitCodes
from numpy.ctypeslib import ndpointer


class BrainFlowMetrics(enum.IntEnum):
    """Enum to store all supported metrics"""

    MINDFULNESS = 0  #:
    RESTFULNESS = 1  #:
    USER_DEFINED = 2  #:


class BrainFlowClassifiers(enum.IntEnum):
    """Enum to store all supported classifiers"""

    DEFAULT_CLASSIFIER = 0  #:
    DYN_LIB_CLASSIFIER = 1  #:
    ONNX_CLASSIFIER = 2  #:


class BrainFlowModelParams(object):
    """ inputs parameters for prepare_session method

    :param metric: metric to calculate
    :type metric: int
    :param classifier: classifier to use
    :type classifier: int
    :param file: file to load model
    :type file: str
    :param other_info: additional information
    :type other_info: str
    :param output_name: output node name
    :type output_name: str
    :param max_array_size: max array size to preallocate
    :type max_array_size: int
    """

    def __init__(self, metric, classifier) -> None:
        self.metric = metric
        self.classifier = classifier
        self.file = ''
        self.other_info = ''
        self.output_name = ''
        self.max_array_size = 8192

    def to_json(self) -> None:
        return json.dumps(self, default=lambda o: o.__dict__,
                          sort_keys=True, indent=4)


class MLModuleDLL(object):
    __instance = None

    @classmethod
    def get_instance(cls):
        if cls.__instance is None:
            cls.__instance = cls()
        return cls.__instance

    def __init__(self):
        if platform.system() == 'Windows':
            if struct.calcsize("P") * 8 == 64:
                dll_path = 'lib\\MLModule.dll'
            else:
                dll_path = 'lib\\MLModule32.dll'
        elif platform.system() == 'Darwin':
            dll_path = 'lib/libMLModule.dylib'
        else:
            dll_path = 'lib/libMLModule.so'
        full_path = pkg_resources.resource_filename(__name__, dll_path)
        if os.path.isfile(full_path):
            # for python we load dll by direct path but this dll may depend on other dlls and they will not be found!
            # to solve it we can load all of them before loading the main one or change PATH\LD_LIBRARY_PATH env var.
            # env variable looks better, since it can be done only once for all dependencies
            dir_path = os.path.abspath(os.path.dirname(full_path))
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

        self.set_log_level_ml_module = self.lib.set_log_level_ml_module
        self.set_log_level_ml_module.restype = ctypes.c_int
        self.set_log_level_ml_module.argtypes = [
            ctypes.c_int
        ]

        self.set_log_file_ml_module = self.lib.set_log_file_ml_module
        self.set_log_file_ml_module.restype = ctypes.c_int
        self.set_log_file_ml_module.argtypes = [
            ctypes.c_char_p
        ]

        self.log_message_ml_module = self.lib.log_message_ml_module
        self.log_message_ml_module.restype = ctypes.c_int
        self.log_message_ml_module.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.prepare = self.lib.prepare
        self.prepare.restype = ctypes.c_int
        self.prepare.argtypes = [
            ctypes.c_char_p
        ]

        self.release = self.lib.release
        self.release.restype = ctypes.c_int
        self.release.argtypes = [
            ctypes.c_char_p
        ]

        self.release_all = self.lib.release_all
        self.release_all.restype = ctypes.c_int
        self.release_all.argtypes = []

        self.predict = self.lib.predict
        self.predict.restype = ctypes.c_int
        self.predict.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_int32),
            ctypes.c_char_p
        ]

        self.get_version_ml_module = self.lib.get_version_ml_module
        self.get_version_ml_module.restype = ctypes.c_int
        self.get_version_ml_module.argtypes = [
            ndpointer(ctypes.c_ubyte),
            ndpointer(ctypes.c_int32),
            ctypes.c_int
        ]


class MLModel(object):
    """MLModel class used to calc derivative metrics from raw data

    :param model_params: Model Params
    :type model_params: BrainFlowModelParams
    """

    def __init__(self, model_params: BrainFlowModelParams) -> None:
        self.model_params = model_params
        try:
            self.serialized_params = model_params.to_json().encode()
        except BaseException:
            self.serialized_params = model_params.to_json()

    @classmethod
    def set_log_level(cls, log_level: int) -> None:
        """set BrainFlow log level, use it only if you want to write your own messages to BrainFlow logger,
        otherwise use enable_ml_logger, enable_dev_ml_logger or disable_ml_logger

        :param log_level: log level, to specify it you should use values from LogLevels enum
        :type log_level: int
        """
        res = MLModuleDLL.get_instance().set_log_level_ml_module(log_level)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to enable logger', res)

    @classmethod
    def enable_ml_logger(cls) -> None:
        """enable ML Logger with level INFO, uses stderr for log messages by default"""
        cls.set_log_level(LogLevels.LEVEL_INFO.value)

    @classmethod
    def disable_ml_logger(cls) -> None:
        """disable BrainFlow Logger"""
        cls.set_log_level(LogLevels.LEVEL_OFF.value)

    @classmethod
    def enable_dev_ml_logger(cls) -> None:
        """enable ML Logger with level TRACE, uses stderr for log messages by default"""
        cls.set_log_level(LogLevels.LEVEL_TRACE.value)

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
        res = MLModuleDLL.get_instance().set_log_file_ml_module(file)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to redirect logs to a file', res)

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
        res = MLModuleDLL.get_instance().log_message_ml_module(log_level, msg)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to write log message', res)

    @classmethod
    def release_all(cls) -> None:
        """release all classifiers"""

        res = MLModuleDLL.get_instance().release_all()
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to release classifiers', res)

    @classmethod
    def get_version(cls) -> str:
        """get version of brainflow libraries

        :return: version
        :rtype: str
        :raises BrainFlowError
        """
        string = numpy.zeros(64).astype(numpy.ubyte)
        string_len = numpy.zeros(1).astype(numpy.int32)
        res = MLModuleDLL.get_instance().get_version_ml_module(string, string_len, 64)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info', res)
        return string.tobytes().decode('utf-8')[0:string_len[0]]

    def prepare(self) -> None:
        """prepare classifier"""

        res = MLModuleDLL.get_instance().prepare(self.serialized_params)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to prepare classifier', res)

    def release(self) -> None:
        """release classifier"""

        res = MLModuleDLL.get_instance().release(self.serialized_params)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to release classifier', res)

    def predict(self, data) -> List:
        """calculate metric from data

        :param data: input array
        :type data: NDArray[Shape["*"], Float64]
        :return: metric value
        :rtype: List
        """
        output = numpy.zeros(self.model_params.max_array_size).astype(numpy.float64)
        output_len = numpy.zeros(1).astype(numpy.int32)
        res = MLModuleDLL.get_instance().predict(data, data.shape[0], output, output_len, self.serialized_params)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calc metric', res)
        return output[0:output_len[0]]
