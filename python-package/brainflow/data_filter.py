import ctypes
import numpy
from numpy.ctypeslib import ndpointer
import pkg_resources
import enum
import os
import platform
import sys
import struct
from typing import List, Set, Dict, Tuple

from nptyping import NDArray, Float64, Complex128

from brainflow.board_shim import BrainFlowError, LogLevels
from brainflow.exit_codes import BrainflowExitCodes


class FilterTypes (enum.Enum):
    """Enum to store all supported Filter Types"""

    BUTTERWORTH = 0 #:
    CHEBYSHEV_TYPE_1 = 1 #:
    BESSEL = 2 #:


class AggOperations (enum.Enum):
    """Enum to store all supported aggregation operations"""

    MEAN = 0 #:
    MEDIAN = 1 #:
    EACH = 2 #:


class WindowFunctions (enum.Enum):
    """Enum to store all supported window functions"""

    NO_WINDOW = 0 #:
    HANNING = 1 #:
    HAMMING = 2 #:
    BLACKMAN_HARRIS = 3 #:


class DetrendOperations (enum.Enum):
    """Enum to store all supported detrend options"""

    NONE = 0 #:
    CONSTANT = 1 #:
    LINEAR = 2 #:


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

        self.write_file = self.lib.write_file
        self.write_file.restype = ctypes.c_int
        self.write_file.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_char_p,
            ctypes.c_char_p
        ]

        self.read_file = self.lib.read_file
        self.read_file.restype = ctypes.c_int
        self.read_file.argtypes = [
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_int32),
            ctypes.c_char_p,
            ctypes.c_int
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

        self.get_num_elements_in_file = self.lib.get_num_elements_in_file
        self.get_num_elements_in_file.restype = ctypes.c_int
        self.get_num_elements_in_file.argtypes = [
            ctypes.c_char_p,
            ndpointer (ctypes.c_int32)
        ]

        self.perform_rolling_filter = self.lib.perform_rolling_filter
        self.perform_rolling_filter.restype = ctypes.c_int
        self.perform_rolling_filter.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int
        ]

        self.perform_downsampling = self.lib.perform_downsampling
        self.perform_downsampling.restype = ctypes.c_int
        self.perform_downsampling.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer (ctypes.c_double)
        ]

        self.perform_wavelet_transform = self.lib.perform_wavelet_transform
        self.perform_wavelet_transform.restype = ctypes.c_int
        self.perform_wavelet_transform.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_char_p,
            ctypes.c_int,
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_int32)
        ]

        self.perform_inverse_wavelet_transform = self.lib.perform_inverse_wavelet_transform
        self.perform_inverse_wavelet_transform.restype = ctypes.c_int
        self.perform_inverse_wavelet_transform.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_char_p,
            ctypes.c_int,
            ndpointer (ctypes.c_int32),
            ndpointer (ctypes.c_double)
        ]

        self.perform_fft = self.lib.perform_fft
        self.perform_fft.restype = ctypes.c_int
        self.perform_fft.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_double)
        ]

        self.perform_ifft = self.lib.perform_ifft
        self.perform_ifft.restype = ctypes.c_int
        self.perform_ifft.argtypes = [
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ndpointer (ctypes.c_double)
        ]

        self.get_nearest_power_of_two = self.lib.get_nearest_power_of_two
        self.get_nearest_power_of_two.restype = ctypes.c_int
        self.get_nearest_power_of_two.argtypes = [
            ctypes.c_int,
            ndpointer (ctypes.c_int32)
        ]

        self.perform_wavelet_denoising = self.lib.perform_wavelet_denoising
        self.perform_wavelet_denoising.restype = ctypes.c_int
        self.perform_wavelet_denoising.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_char_p,
            ctypes.c_int
        ]

        self.get_avg_band_powers = self.lib.get_avg_band_powers
        self.get_avg_band_powers.restype = ctypes.c_int
        self.get_avg_band_powers.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_double),
        ]

        self.get_psd = self.lib.get_psd
        self.get_psd.restype = ctypes.c_int
        self.get_psd.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_double),
        ]

        self.get_psd_welch = self.lib.get_psd_welch
        self.get_psd_welch.restype = ctypes.c_int
        self.get_psd_welch.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_double),
        ]

        self.detrend = self.lib.detrend
        self.detrend.restype = ctypes.c_int
        self.detrend.argtypes = [
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int
        ]

        self.get_band_power = self.lib.get_band_power
        self.get_band_power.restype = ctypes.c_int
        self.get_band_power.argtypes = [
            ndpointer (ctypes.c_double),
            ndpointer (ctypes.c_double),
            ctypes.c_int,
            ctypes.c_double,
            ctypes.c_double,
            ndpointer (ctypes.c_double)
        ]


class DataFilter (object):
    """DataFilter class contains methods for signal processig"""

    @classmethod
    def _set_log_level (cls, log_level: int) -> None:
        """set BrainFlow log level, use it only if you want to write your own messages to BrainFlow logger,
        otherwise use enable_data_logger, enable_dev_data_logger or disable_data_logger

        :param log_level: log level, to specify it you should use values from LogLevels enum
        :type log_level: int
        """
        res = DataHandlerDLL.get_instance ().set_log_level (log_level)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to enable logger', res)

    @classmethod
    def enable_data_logger (cls) -> None:
        """enable Data Logger with level INFO, uses stderr for log messages by default"""
        cls._set_log_level (LogLevels.LEVEL_INFO.value)

    @classmethod
    def disable_data_logger (cls) -> None:
        """disable Data Logger"""
        cls._set_log_level (LogLevels.LEVEL_OFF.value)

    @classmethod
    def enable_dev_data_logger (cls) -> None:
        """enable Data Logger with level TRACE, uses stderr for log messages by default"""
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
        res = DataHandlerDLL.get_instance ().set_log_file (file)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to redirect logs to a file', res)

    @classmethod
    def perform_lowpass (cls, data: NDArray[Float64], sampling_rate: int, cutoff: float, order: int, filter_type: int, ripple: float) -> None:
        """apply low pass filter to provided data

        :param data: data to filter, filter works in-place
        :type data: NDArray[Float64]
        :param sampling_rate: board's sampling rate
        :type sampling_rate: int
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
        if len (data.shape) != 1:
            raise BrainFlowError ('wrong shape for filter data array, it should be 1d array', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().perform_lowpass (data, data.shape[0], sampling_rate, cutoff, order, filter_type, ripple)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to perform low pass filter', res)

    @classmethod
    def perform_highpass (cls, data: NDArray[Float64], sampling_rate: int, cutoff: float, order: int, filter_type: int , ripple: float) -> None:
        """apply high pass filter to provided data

        :param data: data to filter, filter works in-place
        :type data: NDArray[Float64]
        :param sampling_rate: board's sampling rate
        :type sampling_rate: int
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
        if len (data.shape) != 1:
            raise BrainFlowError ('wrong shape for filter data array, it should be 1d array', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().perform_highpass (data, data.shape[0], sampling_rate, cutoff, order, filter_type, ripple)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to apply high pass filter', res)

    @classmethod
    def perform_bandpass (cls, data: NDArray[Float64], sampling_rate: int, center_freq: float,
        band_width: float, order: int, filter_type: int, ripple: float) -> None:
        """apply band pass filter to provided data

        :param data: data to filter, filter works in-place
        :type data: NDArray[Float64]
        :param sampling_rate: board's sampling rate
        :type sampling_rate: int
        :param center_freq: center frequency
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
        if len (data.shape) != 1:
            raise BrainFlowError ('wrong shape for filter data array, it should be 1d array', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().perform_bandpass (data, data.shape[0], sampling_rate, center_freq, band_width, order, filter_type, ripple)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to apply band pass filter', res)

    @classmethod
    def perform_bandstop (cls, data: NDArray[Float64], sampling_rate: int, center_freq: float,
        band_width: float, order: int, filter_type: int, ripple: float) -> None:
        """apply band stop filter to provided data

        :param data: data to filter, filter works in-place
        :type data: NDArray[Float64]
        :param sampling_rate: board's sampling rate
        :type sampling_rate: int
        :param center_freq: center frequency
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
        if len (data.shape) != 1:
            raise BrainFlowError ('wrong shape for filter data array, it should be 1d array', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().perform_bandstop (data, data.shape[0], sampling_rate, center_freq, band_width, order, filter_type, ripple)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to apply band stop filter', res)

    @classmethod
    def perform_rolling_filter (cls, data: NDArray[Float64], period: int, operation: int) -> None:
        """smooth data using moving average or median

        :param data: data to smooth, it works in-place
        :type data: NDArray[Float64]
        :param period: window size
        :type period: int
        :param operation: int value from AggOperation enum
        :type operation: int
        """
        if not isinstance (period, int):
            raise BrainFlowError ('wrong type for period', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance (operation, int):
            raise BrainFlowError ('wrong type for operation', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if len (data.shape) != 1:
            raise BrainFlowError ('wrong shape for filter data array, it should be 1d array', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().perform_rolling_filter (data, data.shape[0], period, operation)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to smooth data', res)

    @classmethod
    def perform_downsampling (cls, data: NDArray[Float64], period: int, operation: int) -> NDArray[Float64]:
        """perform data downsampling, it doesnt apply lowpass filter for you, it just aggregates several data points

        :param data: initial data
        :type data: NDArray[Float64]
        :param period: downsampling period
        :type period: int
        :param operation: int value from AggOperation enum
        :type operation: int
        :return: downsampled data
        :rtype: NDArray[Float64]
        """
        if not isinstance (period, int):
            raise BrainFlowError ('wrong type for period', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance (operation, int):
            raise BrainFlowError ('wrong type for operation', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if len (data.shape) != 1:
            raise BrainFlowError ('wrong shape for filter data array, it should be 1d array', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if period <= 0:
            raise BrainFlowError ('Invalid value for period', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)

        downsampled_data = numpy.zeros (int (data.shape[0] / period)).astype (numpy.float64)
        res = DataHandlerDLL.get_instance ().perform_downsampling (data, data.shape[0], period, operation, downsampled_data)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to perform downsampling', res)

        return downsampled_data

    @classmethod
    def perform_wavelet_transform (cls, data: NDArray[Float64], wavelet: str, decomposition_level: int) -> Tuple:
        """perform wavelet transform

        :param data: initial data
        :type data: NDArray[Float64]
        :param wavelet: supported vals: db1..db15,haar,sym2..sym10,coif1..coif5,bior1.1,bior1.3,bior1.5,bior2.2,bior2.4,bior2.6,bior2.8,bior3.1,bior3.3,bior3.5 ,bior3.7,bior3.9,bior4.4,bior5.5,bior6.8
        :type wavelet: str
        :param decomposition_level: level of decomposition
        :type decomposition_level: int
        :return: tuple of wavelet coeffs in format [A(J) D(J) D(J-1) ..... D(1)] where J is decomposition level, A - app coeffs, D - detailed coeffs, and array with lengths for each block
        :rtype: tuple
        """
        try:
            wavelet_func = wavelet.encode ()
        except:
            wavelet_func = wavelet

        wavelet_coeffs = numpy.zeros (data.shape[0] + 2 * (40 + 1)).astype (numpy.float64)
        lengths = numpy.zeros (decomposition_level + 1).astype (numpy.int32)
        res = DataHandlerDLL.get_instance ().perform_wavelet_transform (data, data.shape[0], wavelet_func, decomposition_level, wavelet_coeffs, lengths)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to perform wavelet transform', res)

        return wavelet_coeffs[0: sum (lengths)], lengths

    @classmethod
    def perform_inverse_wavelet_transform (cls, wavelet_output: Tuple, original_data_len: int, wavelet: str, decomposition_level: int) -> NDArray[Float64]:
        """perform wavelet transform

        :param wavelet_output: tuple of wavelet_coeffs and array with lengths
        :type wavelet_coeffs: tuple
        :param original_data_len: len of signal before wavelet transform
        :type original_data_len: int
        :param wavelet: supported vals: db1..db15,haar,sym2..sym10,coif1..coif5,bior1.1,bior1.3,bior1.5,bior2.2,bior2.4,bior2.6,bior2.8,bior3.1,bior3.3,bior3.5 ,bior3.7,bior3.9,bior4.4,bior5.5,bior6.8
        :type wavelet: str
        :param decomposition_level: level of decomposition
        :type decomposition_level: int
        :return: restored data
        :rtype: NDArray[Float64]
        """
        try:
            wavelet_func = wavelet.encode ()
        except:
            wavelet_func = wavelet

        original_data = numpy.zeros (original_data_len).astype (numpy.float64)
        res = DataHandlerDLL.get_instance ().perform_inverse_wavelet_transform (wavelet_output[0], original_data_len, wavelet_func, 
                                                                                decomposition_level, wavelet_output[1], original_data)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to perform inverse wavelet transform', res)

        return original_data

    @classmethod
    def perform_wavelet_denoising (cls, data: NDArray[Float64], wavelet: str, decomposition_level: int) -> None:
        """perform wavelet denoising

        :param data: data to denoise
        :type data: NDArray[Float64]
        :param wavelet: supported vals: db1..db15,haar,sym2..sym10,coif1..coif5,bior1.1,bior1.3,bior1.5,bior2.2,bior2.4,bior2.6,bior2.8,bior3.1,bior3.3,bior3.5 ,bior3.7,bior3.9,bior4.4,bior5.5,bior6.8
        :type wavelet: str
        :param decomposition_level: decomposition level
        :type decomposition_level: int
        """
        try:
            wavelet_func = wavelet.encode ()
        except:
            wavelet_func = wavelet

        res = DataHandlerDLL.get_instance ().perform_wavelet_denoising (data, data.shape[0], wavelet_func, decomposition_level)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to denoise data', res)

    @classmethod
    def perform_fft (cls, data: NDArray[Float64], window: int) -> NDArray[Complex128]:
        """perform direct fft

        :param data: data for fft, len of data must be a power of 2
        :type data: NDArray[Float64]
        :param window: window function
        :type window: int
        :return: numpy array of complex values, len of this array is N / 2 + 1
        :rtype: NDArray[Complex128]
        """
        def is_power_of_two (n):
            return (n != 0) and (n & (n - 1) == 0)

        if (not is_power_of_two (data.shape[0])):
            raise BrainFlowError ('data len is not power of 2: %d' % data.shape[0], BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)

        temp_re = numpy.zeros (int (data.shape[0] / 2 + 1)).astype (numpy.float64)
        temp_im = numpy.zeros (int (data.shape[0] / 2 + 1)).astype (numpy.float64)
        res = DataHandlerDLL.get_instance ().perform_fft (data, data.shape[0], window, temp_re, temp_im)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to perform fft', res)

        output = numpy.zeros (int (data.shape[0] / 2 + 1)).astype (numpy.complex128)
        for i in range (output.shape[0]):
            output[i] = numpy.complex128 (complex (temp_re[i], temp_im[i]))

        return output

    @classmethod
    def get_psd (cls, data: NDArray[Float64], sampling_rate: int, window: int) -> Tuple:
        """calculate PSD

        :param data: data to calc psd, len of data must be a power of 2
        :type data: NDArray[Float64]
        :param sampling_rate: sampling rate
        :type sampling_rate: int
        :param window: window function
        :type window: int
        :return: amplitude and frequency arrays of len N / 2 + 1
        :rtype: tuple
        """
        def is_power_of_two (n):
            return (n != 0) and (n & (n - 1) == 0)

        if (not is_power_of_two (data.shape[0])):
            raise BrainFlowError ('data len is not power of 2: %d' % data.shape[0], BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)

        ampls = numpy.zeros (int (data.shape[0] / 2 + 1)).astype (numpy.float64)
        freqs = numpy.zeros (int (data.shape[0] / 2 + 1)).astype (numpy.float64)
        res = DataHandlerDLL.get_instance ().get_psd (data, data.shape[0], sampling_rate, window, ampls, freqs)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to calc psd', res)

        return ampls, freqs

    @classmethod
    def get_psd_welch (cls, data: NDArray[Float64], nfft: int, overlap: int, sampling_rate: int, window: int) -> Tuple:
        """calculate PSD using Welch method

        :param data: data to calc psd
        :type data: NDArray[Float64]
        :param nfft: FFT Window size, must be power of 2
        :type nfft: int
        :param overlap: overlap of FFT Windows, must be between 0 and nfft
        :type overlap: int
        :param sampling_rate: sampling rate
        :type sampling_rate: int
        :param window: window function
        :type window: int
        :return: amplitude and frequency arrays of len N / 2 + 1
        :rtype: tuple
        """
        def is_power_of_two (n):
            return (n != 0) and (n & (n - 1) == 0)

        if (not is_power_of_two (nfft)):
            raise BrainFlowError ('nfft is not power of 2: %d' % nfft, BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)

        ampls = numpy.zeros (int (nfft / 2 + 1)).astype (numpy.float64)
        freqs = numpy.zeros (int (nfft / 2 + 1)).astype (numpy.float64)
        res = DataHandlerDLL.get_instance ().get_psd_welch (data, data.shape[0], nfft, overlap, sampling_rate, window, ampls, freqs)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to calc psd welch', res)

        return ampls, freqs

    @classmethod
    def detrend (cls, data: NDArray[Float64], detrend_operation: int) -> None:
        """detrend data

        :param data: data to calc psd
        :type data: NDArray[Float64]
        :param detrend_operation: Type of detrend operation
        :type detrend_operation: int
        """
        if len (data.shape) != 1:
            raise BrainFlowError ('wrong shape for data, should be 1d array', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance ().detrend (data, data.shape[0], detrend_operation)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to detrend data', res)

    @classmethod
    def get_band_power (cls, psd: Tuple, freq_start: float, freq_end: float) -> float:
        """calculate band power

        :param psd: psd from get_psd
        :type psd: typle
        :param freq_start: start freq
        :type freq_start: int
        :param freq_end: end freq
        :type freq_end: int
        :return: band power
        :rtype: float
        """
        band_power = numpy.zeros (1).astype (numpy.float64)
        res = DataHandlerDLL.get_instance ().get_band_power (psd[0], psd[1], psd[0].shape[0], freq_start, freq_end, band_power)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to calc band power', res)

        return band_power[0]

    @classmethod
    def get_avg_band_powers (cls, data: NDArray, channels: List, sampling_rate: int, apply_filter: bool) -> Tuple:
        """calculate avg and stddev of BandPowers across all channels

        :param data: 2d array for calculation
        :type data: NDArray
        :param channels: channels - rows of data array which should be used for calculation
        :type channels: List
        :param sampling_rate: sampling rate
        :type sampling_rate: int
        :param apply_filter: apply bandpass and bandstop filtrers or not
        :type apply_filter: bool
        :return: avg and stddev arrays for bandpowers
        :rtype: tuple
        """

        if (data.ndim != 2):
            raise BrainFlowError ('Shape of data array must be 2', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)

        avg_bands = numpy.zeros (5).astype (numpy.float64)
        stddev_bands = numpy.zeros (5).astype (numpy.float64)
        data_1d = numpy.zeros (len (channels) * data.shape[1])
        for i, channel in enumerate (channels):
            for j in range (data.shape[1]):
                data_1d[j + data.shape[1] * i] = data[channel][j]
        res = DataHandlerDLL.get_instance ().get_avg_band_powers (data_1d, len (channels), data.shape[1], sampling_rate,
            int (apply_filter), avg_bands, stddev_bands)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to get_avg_band_powers', res)

        return avg_bands, stddev_bands

    @classmethod
    def perform_ifft (cls, data: NDArray[Complex128]) -> NDArray[Float64]:
        """perform inverse fft

        :param data: data from fft
        :type data: NDArray[Complex128]
        :return: restored data
        :rtype: NDArray[Float64]
        """
        temp_re = numpy.zeros (data.shape[0]).astype (numpy.float64)
        temp_im = numpy.zeros (data.shape[0]).astype (numpy.float64)
        for i in range (data.shape[0]):
            temp_re[i] = data[i].real
            temp_im[i] = data[i].imag
        output = numpy.zeros (2 * (data.shape[0] - 1)).astype (numpy.float64)

        res = DataHandlerDLL.get_instance ().perform_ifft (temp_re, temp_im, output.shape[0], output)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to perform ifft', res)

        return output

    @classmethod
    def get_nearest_power_of_two (cls, value: int) -> int:
        """calc nearest power of two

        :param value: input value
        :type value: int
        :return: nearest power of two
        :rtype: int
        """
        output = numpy.zeros (1).astype (numpy.int32)
        res = DataHandlerDLL.get_instance ().get_nearest_power_of_two (value, output)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to calc nearest power of two', res)

        return output[0]

    @classmethod
    def write_file (cls, data, file_name: str, file_mode: str) -> None:
        """write data to file, in file data will be transposed

        :param data: data to store in a file
        :type data: 2d numpy array
        :param file_name: file name to store data
        :type file_name: str
        :param file_mode: 'w' to rewrite file or 'a' to append data to file
        :type file_mode: str
        """
        if len (data.shape) != 2:
            raise BrainFlowError ('wrong shape for filter data array, it should be 2d array', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        try:
            file = file_name.encode ()
        except:
            file = file_name
        try:
            mode = file_mode.encode ()
        except:
            mode = file_mode
        data_flatten = data.flatten ()
        res = DataHandlerDLL.get_instance ().write_file (data_flatten, data.shape[0], data.shape[1], file, mode)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to write file', res)

    @classmethod
    def read_file (cls, file_name: str):
        """read data from file

        :param file_name: file name to read
        :type file_name: str
        :return: 2d numpy array with data from this file, data will be transposed to original dimensions
        :rtype: 2d numpy array
        """
        try:
            file = file_name.encode ()
        except:
            file = file_name

        num_elements = numpy.zeros (1).astype (numpy.int32)
        res = DataHandlerDLL.get_instance ().get_num_elements_in_file (file, num_elements)
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to determine number of elements in file', res)

        data_arr = numpy.zeros (num_elements[0]).astype (numpy.float64)
        num_rows = numpy.zeros (1).astype (numpy.int32)
        num_cols = numpy.zeros (1).astype (numpy.int32)

        res = DataHandlerDLL.get_instance ().read_file (data_arr, num_rows, num_cols, file, num_elements[0])
        if res != BrainflowExitCodes.STATUS_OK.value:
            raise BrainFlowError ('unable to read file', res)

        if len (num_rows) == 0 or len (num_cols) == 0:
            return None

        data_arr = data_arr[0:num_rows[0] * num_cols[0]].reshape (num_rows[0], num_cols[0])
        return data_arr        
