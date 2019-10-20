import ctypes
import numpy
from numpy.ctypeslib import ndpointer
import pkg_resources
import enum
import os
import platform
import sys
import struct

from brainflow.board_shim import BrainFlowError
from brainflow.exit_codes import BrainflowExitCodes


class FilterTypes (enum.Enum):
    """Enum to store all supported Filter Types"""

    BUTTERWORTH = 0 #:
    CHEBYSHEV_TYPE_1 = 1 #:
    BESSEL = 2 #:


class DataHandlerDLL (object):

    __instance = None

    @classmethod
    def get_instance (cls):
        if cls.__instance is None:
            cls.__instance = cls ()
        return cls.__instance

    def __init__ (self):
        if platform.system () == 'Windows':
            if struct.calcsize ("P") * 8 == 64:
                dll_path = 'lib\\DataHandler.dll'
            else:
                dll_path = 'lib\\DataHandler32.dll'
        elif platform.system () == 'Darwin':
            dll_path = 'lib/libDataHandler.dylib'
        else:
            dll_path = 'lib/libDataHandler.so'
        full_path = pkg_resources.resource_filename (__name__, dll_path)
        if os.path.isfile (full_path):
            self.lib = ctypes.cdll.LoadLibrary (full_path)
        else:
            raise FileNotFoundError ('Dynamic library %s is missed, did you forget to compile brainflow before installation of python package?' % full_path)

        self.perform_lowpass = self.lib.perform_lowpass
        self.perform_lowpass.restype = ctypes.c_int
        self.perform_lowpass.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double
        ]

        self.perform_highpass = self.lib.perform_highpass
        self.perform_highpass.restype = ctypes.c_int
        self.perform_highpass.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double
        ]

        self.perform_bandpass = self.lib.perform_bandpass
        self.perform_bandpass.restype = ctypes.c_int
        self.perform_bandpass.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double,
            ctypes.c_double,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double
        ]

        self.perform_bandstop = self.lib.perform_bandstop
        self.perform_bandstop.restype = ctypes.c_int
        self.perform_bandstop.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double,
            ctypes.c_double,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double
        ]


class DataFilter (object):
    """DataFilter class contains methods for signal processig"""

    @classmethod
    def perform_lowpass (cls, data, sampling_rate, cutoff, order, filter_type, ripple):
        """apply low pass filter to provided data

        :param data: data to filter, filter works in-place
        :type data: numpy array
        :param sampling_rate: board's sampling rate
        :type sampling_rate: float
        :param cutoff: cutoff frequency
        :type cutoff: float
        :param order: filter order
        :type order: int
        :param filter_type: filter type from special enum
        :type filter_type: int
        :param ripple: ripple value for Chebyshev filter
        :type ripple: float
        """
        if not isinstance (sampling_rate, int):
            raise BrainFlowError ('wrong type for sampling rate', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance (filter_type, int):
            raise BrainFlowError ('wrong type for filter type', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().perform_lowpass (data, data.shape[0], sampling_rate, cutoff, order, filter_type, ripple)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to perform low pass filter', res)

    @classmethod
    def perform_highpass (cls, data, sampling_rate, cutoff, order, filter_type, ripple):
        """apply high pass filter to provided data

        :param data: data to filter, filter works in-place
        :type data: numpy array
        :param sampling_rate: board's sampling rate
        :type sampling_rate: float
        :param cutoff: cutoff frequency
        :type cutoff: float
        :param order: filter order
        :type order: int
        :param filter_type: filter type from special enum
        :type filter_type: int
        :param ripple: ripple value for Chebyshev filter
        :type ripple: float
        """
        if not isinstance (sampling_rate, int):
            raise BrainFlowError ('wrong type for sampling rate', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance (filter_type, int):
            raise BrainFlowError ('wrong type for filter type', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().perform_highpass (data, data.shape[0], sampling_rate, cutoff, order, filter_type, ripple)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to apply high pass filter', res)

    @classmethod
    def perform_bandpass (cls, data, sampling_rate, center_freq, band_width, order, filter_type, ripple):
        """apply band pass filter to provided data

        :param data: data to filter, filter works in-place
        :type data: numpy array
        :param sampling_rate: board's sampling rate
        :type sampling_rate: float
        :param center_freq: center frequency frequency
        :type center_freq: float
        :param band_width: band width
        :type band_width: float
        :param order: filter order
        :type order: int
        :param filter_type: filter type from special enum
        :type filter_type: int
        :param ripple: ripple value for Chebyshev filter
        :type ripple: float
        """
        if not isinstance (sampling_rate, int):
            raise BrainFlowError ('wrong type for sampling rate', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance (filter_type, int):
            raise BrainFlowError ('wrong type for filter type', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().perform_bandpass (data, data.shape[0], sampling_rate, center_freq, band_width, order, filter_type, ripple)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to apply band pass filter', res)

    @classmethod
    def perform_bandstop (cls, data, sampling_rate, center_freq, band_width, order, filter_type, ripple):
        """apply band stop filter to provided data

        :param data: data to filter, filter works in-place
        :type data: numpy array
        :param sampling_rate: board's sampling rate
        :type sampling_rate: float
        :param center_freq: center frequency frequency
        :type center_freq: float
        :param band_width: band width
        :type band_width: float
        :param order: filter order
        :type order: int
        :param filter_type: filter type from special enum
        :type filter_type: int
        :param ripple: ripple value for Chebyshev filter
        :type ripple: float
        """
        if not isinstance (sampling_rate, int):
            raise BrainFlowError ('wrong type for sampling rate', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance (filter_type, int):
            raise BrainFlowError ('wrong type for filter type', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().perform_bandstop (data, data.shape[0], sampling_rate, center_freq, band_width, order, filter_type, ripple)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to apply band stop filter', res)
