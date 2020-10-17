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
from typing import List, Set, Dict, Tuple

from nptyping import NDArray, Float64

from brainflow.board_shim import BrainFlowError, LogLevels
from brainflow.exit_codes import BrainflowExitCodes


class BrainFlowMetrics (enum.Enum):
    """Enum to store all supported metrics"""

    RELAXATION = 0 #:
    CONCENTRATION = 1 #:


class BrainFlowClassifiers (enum.Enum):
    """Enum to store all supported classifiers"""

    REGRESSION = 0 #:
    KNN = 1 #:
    SVM = 2 #:
    LDA = 3 #:

class BrainFlowModelParams (object):
    """ inputs parameters for prepare_session method

    :param metric: metric to calculate
    :type metric: int
    :param classifier: classifier to use
    :type classifier: int
    :param file: file to load model
    :type file: str
    :param other_info: additional information
    :type other_info: int
    """
    def __init__ (self, metric, classifier) -> None:
        self.metric = metric
        self.classifier = classifier
        self.file = ''
        self.other_info = ''

    def to_json (self) -> None :
        return json.dumps (self, default = lambda o: o.__dict__,
            sort_keys = True, indent = 4)


class MLModuleDLL (object):

    __instance = None

    @classmethod
    def get_instance (cls):
        if cls.__instance is None:
            cls.__instance = cls ()
        return cls.__instance

    def __init__ (self):
        if platform.system () == 'Windows':
            if struct.calcsize ("P") * 8 == 64:
                dll_path = 'lib\\MLModule.dll'
            else:
                dll_path = 'lib\\MLModule32.dll'
        elif platform.system () == 'Darwin':
            dll_path = 'lib/libMLModule.dylib'
        else:
            dll_path = 'lib/libMLModule.so'
        full_path = pkg_resources.resource_filename (__name__, dll_path)
        if os.path.isfile (full_path):
            # for python we load dll by direct path but this dll may depend on other dlls and they will not be found!
            # to solve it we can load all of them before loading the main one or change PATH\LD_LIBRARY_PATH env var.
            # env variable looks better, since it can be done only once for all dependencies
            dir_path = os.path.abspath (os.path.dirname (full_path))
            if platform.system () == 'Windows':
                os.environ['PATH'] = dir_path + os.pathsep + os.environ.get ('PATH', '')
            else:
                os.environ['LD_LIBRARY_PATH'] = dir_path + os.pathsep + os.environ.get ('LD_LIBRARY_PATH', '')
            self.lib = ctypes.cdll.LoadLibrary (full_path)
        else:
            raise FileNotFoundError ('Dynamic library %s is missed, did you forget to compile brainflow before installation of python package?' % full_path)
        
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

        self.predict = self.lib.predict
        self.predict.restype = ctypes.c_int
        self.predict.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ndpointer (ctypes.c_double),
            ctypes.c_char_p
        ]


class MLModel (object):
    """MLModel class used to calc derivative metrics from raw data

    :param model_params: Model Params
    :type model_params: BrainFlowModelParams
    """
    def __init__ (self, model_params : BrainFlowModelParams) -> None:
        self.model_params = model_params
        try:
            self.serialized_params = model_params.to_json ().encode ()
        except:
            self.serialized_params = model_params.to_json ()

    @classmethod
    def _set_log_level (cls, log_level: int) -> None:
        """set BrainFlow log level, use it only if you want to write your own messages to BrainFlow logger,
        otherwise use enable_ml_logger, enable_dev_ml_logger or disable_ml_logger

        :param log_level: log level, to specify it you should use values from LogLevels enum
        :type log_level: int
        """
        res = MLModuleDLL.get_instance ().set_log_level (log_level)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to enable logger', res)

    @classmethod
    def enable_ml_logger (cls) -> None:
        """enable ML Logger with level INFO, uses stderr for log messages by default"""
        cls._set_log_level (LogLevels.LEVEL_INFO.value)

    @classmethod
    def disable_ml_logger (cls) -> None:
        """disable BrainFlow Logger"""
        cls._set_log_level (LogLevels.LEVEL_OFF.value)

    @classmethod
    def enable_dev_ml_logger (cls) -> None:
        """enable ML Logger with level TRACE, uses stderr for log messages by default"""
        cls._set_log_level (LogLevels.LEVEL_TRACE.value)

    @classmethod
    def set_log_file (cls, log_file: str) -> None:
        """redirect logger from stderr to file, can be called any time

        :param log_file: log file name
        :type log_file: str
        """
        try:
            file = log_file.encode ()
        except:
            file = log_file
        res = MLModuleDLL.get_instance ().set_log_file (file)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to redirect logs to a file', res)

    def prepare (self) -> None:
        """prepare classifier"""

        res = MLModuleDLL.get_instance ().prepare (self.serialized_params)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to prepare classifier', res)

    def release (self) -> None:
        """release classifier"""

        res = MLModuleDLL.get_instance ().release (self.serialized_params)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to release classifier', res)

    def predict (self, data: NDArray) -> float:
        """calculate metric from data

        :param data: input array
        :type data: NDArray
        :return: metric value
        :rtype: float
        """
        output = numpy.zeros (1).astype (numpy.float64)
        res = MLModuleDLL.get_instance ().predict (data, data.shape[0], output, self.serialized_params)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to calc metric', res)
        return output[0]
