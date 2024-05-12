import ctypes
import enum
import os
import platform
import struct
from typing import List, Tuple

import numpy
import pkg_resources
from brainflow.exit_codes import BrainFlowExitCodes, BrainFlowError
from brainflow.utils import check_memory_layout_row_major, LogLevels
from numpy.ctypeslib import ndpointer


class FilterTypes(enum.IntEnum):
    """Enum to store all supported Filter Types"""

    BUTTERWORTH = 0  #:
    CHEBYSHEV_TYPE_1 = 1  #:
    BESSEL = 2  #:
    BUTTERWORTH_ZERO_PHASE = 3  #:
    CHEBYSHEV_TYPE_1_ZERO_PHASE = 4  #:
    BESSEL_ZERO_PHASE = 5  #:


class AggOperations(enum.IntEnum):
    """Enum to store all supported aggregation operations"""

    MEAN = 0  #:
    MEDIAN = 1  #:
    EACH = 2  #:


class WindowOperations(enum.IntEnum):
    """Enum to store all supported window functions"""

    NO_WINDOW = 0  #:
    HANNING = 1  #:
    HAMMING = 2  #:
    BLACKMAN_HARRIS = 3  #:


class DetrendOperations(enum.IntEnum):
    """Enum to store all supported detrend options"""

    NO_DETREND = 0  #:
    CONSTANT = 1  #:
    LINEAR = 2  #:


class NoiseTypes(enum.IntEnum):
    """Enum to store noise types"""

    FIFTY = 0  #:
    SIXTY = 1  #:
    FIFTY_AND_SIXTY = 2  #:


class WaveletDenoisingTypes(enum.IntEnum):
    """Enum to store all supported wavelet denoising methods"""

    VISUSHRINK = 0  #:
    SURESHRINK = 1  #:


class ThresholdTypes(enum.IntEnum):
    """Enum to store all supported thresholding types"""

    SOFT = 0  #:
    HARD = 1  #:


class WaveletExtensionTypes(enum.IntEnum):
    """Enum to store all supported wavelet extension types"""

    SYMMETRIC = 0  #:
    PERIODIC = 1  #:


class NoiseEstimationLevelTypes(enum.IntEnum):
    """Enum to store all supported values for noise estemation levels in wavelet denoising"""

    FIRST_LEVEL = 0  #:
    ALL_LEVELS = 1  #:


class WaveletTypes(enum.IntEnum):
    """Enum to store all supported wavelets"""

    HAAR = 0  #:
    DB1 = 1  #:
    DB2 = 2  #:
    DB3 = 3  #:
    DB4 = 4  #:
    DB5 = 5  #:
    DB6 = 6  #:
    DB7 = 7  #:
    DB8 = 8  #:
    DB9 = 9  #:
    DB10 = 10  #:
    DB11 = 11  #:
    DB12 = 12  #:
    DB13 = 13  #:
    DB14 = 14  #:
    DB15 = 15  #:
    BIOR1_1 = 16  #:
    BIOR1_3 = 17  #:
    BIOR1_5 = 18  #:
    BIOR2_2 = 19  #:
    BIOR2_4 = 20  #:
    BIOR2_6 = 21  #:
    BIOR2_8 = 22  #:
    BIOR3_1 = 23  #:
    BIOR3_3 = 24  #:
    BIOR3_5 = 25  #:
    BIOR3_7 = 26  #:
    BIOR3_9 = 27  #:
    BIOR4_4 = 28  #:
    BIOR5_5 = 29  #:
    BIOR6_8 = 30  #:
    COIF1 = 31  #:
    COIF2 = 32  #:
    COIF3 = 33  #:
    COIF4 = 34  #:
    COIF5 = 35  #:
    SYM2 = 36  #:
    SYM3 = 37  #:
    SYM4 = 38  #:
    SYM5 = 39  #:
    SYM6 = 40  #:
    SYM7 = 41  #:
    SYM8 = 42  #:
    SYM9 = 43  #:
    SYM10 = 44  #:


class DataHandlerDLL(object):
    __instance = None

    @classmethod
    def get_instance(cls):
        if cls.__instance is None:
            cls.__instance = cls()
        return cls.__instance

    def __init__(self):
        if platform.system() == 'Windows':
            if struct.calcsize("P") * 8 == 64:
                dll_path = 'lib\\DataHandler.dll'
            else:
                dll_path = 'lib\\DataHandler32.dll'
        elif platform.system() == 'Darwin':
            dll_path = 'lib/libDataHandler.dylib'
        else:
            dll_path = 'lib/libDataHandler.so'
        full_path = pkg_resources.resource_filename(__name__, dll_path)
        if os.path.isfile(full_path):
            dir_path = os.path.abspath(os.path.dirname(full_path))
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

        self.perform_lowpass = self.lib.perform_lowpass
        self.perform_lowpass.restype = ctypes.c_int
        self.perform_lowpass.argtypes = [
            ndpointer(ctypes.c_double),
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
            ndpointer(ctypes.c_double),
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
            ndpointer(ctypes.c_double),
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
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double,
            ctypes.c_double,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double
        ]

        self.get_oxygen_level = self.lib.get_oxygen_level
        self.get_oxygen_level.restype = ctypes.c_int
        self.get_oxygen_level.argtypes = [
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double,
            ctypes.c_double,
            ctypes.c_double,
            ndpointer(ctypes.c_double)
        ]

        self.get_heart_rate = self.lib.get_heart_rate
        self.get_heart_rate.restype = ctypes.c_int
        self.get_heart_rate.argtypes = [
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double)
        ]

        self.log_message_data_handler = self.lib.log_message_data_handler
        self.log_message_data_handler.restype = ctypes.c_int
        self.log_message_data_handler.argtypes = [
            ctypes.c_int,
            ctypes.c_char_p
        ]

        self.remove_environmental_noise = self.lib.remove_environmental_noise
        self.remove_environmental_noise.restype = ctypes.c_int
        self.remove_environmental_noise.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int
        ]

        self.write_file = self.lib.write_file
        self.write_file.restype = ctypes.c_int
        self.write_file.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_char_p,
            ctypes.c_char_p
        ]

        self.read_file = self.lib.read_file
        self.read_file.restype = ctypes.c_int
        self.read_file.argtypes = [
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_int32),
            ctypes.c_char_p,
            ctypes.c_int
        ]

        self.calc_stddev = self.lib.calc_stddev
        self.calc_stddev.restype = ctypes.c_int
        self.calc_stddev.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double)
        ]

        self.get_railed_percentage = self.lib.get_railed_percentage
        self.get_railed_percentage.restype = ctypes.c_int
        self.get_railed_percentage.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double)
        ]

        self.set_log_level_data_handler = self.lib.set_log_level_data_handler
        self.set_log_level_data_handler.restype = ctypes.c_int
        self.set_log_level_data_handler.argtypes = [
            ctypes.c_int
        ]

        self.set_log_file_data_handler = self.lib.set_log_file_data_handler
        self.set_log_file_data_handler.restype = ctypes.c_int
        self.set_log_file_data_handler.argtypes = [
            ctypes.c_char_p
        ]

        self.get_num_elements_in_file = self.lib.get_num_elements_in_file
        self.get_num_elements_in_file.restype = ctypes.c_int
        self.get_num_elements_in_file.argtypes = [
            ctypes.c_char_p,
            ndpointer(ctypes.c_int32)
        ]

        self.perform_rolling_filter = self.lib.perform_rolling_filter
        self.perform_rolling_filter.restype = ctypes.c_int
        self.perform_rolling_filter.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int
        ]

        self.perform_downsampling = self.lib.perform_downsampling
        self.perform_downsampling.restype = ctypes.c_int
        self.perform_downsampling.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double)
        ]

        self.perform_wavelet_transform = self.lib.perform_wavelet_transform
        self.perform_wavelet_transform.restype = ctypes.c_int
        self.perform_wavelet_transform.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_int32)
        ]

        self.detect_peaks_z_score = self.lib.detect_peaks_z_score
        self.detect_peaks_z_score.restype = ctypes.c_int
        self.detect_peaks_z_score.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_double,
            ctypes.c_double,
            ndpointer(ctypes.c_double)
        ]

        self.restore_data_from_wavelet_detailed_coeffs = self.lib.restore_data_from_wavelet_detailed_coeffs
        self.restore_data_from_wavelet_detailed_coeffs.restype = ctypes.c_int
        self.restore_data_from_wavelet_detailed_coeffs.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double)
        ]

        self.perform_inverse_wavelet_transform = self.lib.perform_inverse_wavelet_transform
        self.perform_inverse_wavelet_transform.restype = ctypes.c_int
        self.perform_inverse_wavelet_transform.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_int32),
            ndpointer(ctypes.c_double)
        ]

        self.get_csp = self.lib.get_csp
        self.get_csp.restype = ctypes.c_int
        self.get_csp.argtypes = [
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double)
        ]

        self.get_window = self.lib.get_window
        self.get_window.restype = ctypes.c_int
        self.get_window.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double)
        ]

        self.perform_fft = self.lib.perform_fft
        self.perform_fft.restype = ctypes.c_int
        self.perform_fft.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double)
        ]

        self.perform_ifft = self.lib.perform_ifft
        self.perform_ifft.restype = ctypes.c_int
        self.perform_ifft.argtypes = [
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ndpointer(ctypes.c_double)
        ]

        self.get_nearest_power_of_two = self.lib.get_nearest_power_of_two
        self.get_nearest_power_of_two.restype = ctypes.c_int
        self.get_nearest_power_of_two.argtypes = [
            ctypes.c_int,
            ndpointer(ctypes.c_int32)
        ]

        self.perform_wavelet_denoising = self.lib.perform_wavelet_denoising
        self.perform_wavelet_denoising.restype = ctypes.c_int
        self.perform_wavelet_denoising.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int
        ]

        self.get_custom_band_powers = self.lib.get_custom_band_powers
        self.get_custom_band_powers.restype = ctypes.c_int
        self.get_custom_band_powers.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_float),
            ndpointer(ctypes.c_float),
        ]

        self.perform_ica = self.lib.perform_ica
        self.perform_ica.restype = ctypes.c_int
        self.perform_ica.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double)
        ]

        self.get_custom_band_powers = self.lib.get_custom_band_powers
        self.get_custom_band_powers.restype = ctypes.c_int
        self.get_custom_band_powers.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
        ]

        self.get_psd = self.lib.get_psd
        self.get_psd.restype = ctypes.c_int
        self.get_psd.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
        ]

        self.get_psd_welch = self.lib.get_psd_welch
        self.get_psd_welch.restype = ctypes.c_int
        self.get_psd_welch.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
        ]

        self.detrend = self.lib.detrend
        self.detrend.restype = ctypes.c_int
        self.detrend.argtypes = [
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_int
        ]

        self.get_band_power = self.lib.get_band_power
        self.get_band_power.restype = ctypes.c_int
        self.get_band_power.argtypes = [
            ndpointer(ctypes.c_double),
            ndpointer(ctypes.c_double),
            ctypes.c_int,
            ctypes.c_double,
            ctypes.c_double,
            ndpointer(ctypes.c_double)
        ]

        self.get_version_data_handler = self.lib.get_version_data_handler
        self.get_version_data_handler.restype = ctypes.c_int
        self.get_version_data_handler.argtypes = [
            ndpointer(ctypes.c_ubyte),
            ndpointer(ctypes.c_int32),
            ctypes.c_int
        ]


class DataFilter(object):
    """DataFilter class contains methods for signal processig"""

    @classmethod
    def set_log_level(cls, log_level: int) -> None:
        """set BrainFlow log level, use it only if you want to write your own messages to BrainFlow logger,
        otherwise use enable_data_logger, enable_dev_data_logger or disable_data_logger

        :param log_level: log level, to specify it you should use values from LogLevels enum
        :type log_level: int
        """
        res = DataHandlerDLL.get_instance().set_log_level_data_handler(log_level)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to enable logger', res)

    @classmethod
    def enable_data_logger(cls) -> None:
        """enable Data Logger with level INFO, uses stderr for log messages by default"""
        cls.set_log_level(LogLevels.LEVEL_INFO.value)

    @classmethod
    def disable_data_logger(cls) -> None:
        """disable Data Logger"""
        cls.set_log_level(LogLevels.LEVEL_OFF.value)

    @classmethod
    def enable_dev_data_logger(cls) -> None:
        """enable Data Logger with level TRACE, uses stderr for log messages by default"""
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
        res = DataHandlerDLL.get_instance().set_log_file_data_handler(file)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to redirect logs to a file', res)

    @classmethod
    def perform_lowpass(cls, data, sampling_rate: int, cutoff: float, order: int, filter_type: int,
                        ripple: float) -> None:
        """apply low pass filter to provided data

        :param data: data to filter, filter works in-place
        :type data: NDArray[Shape["*"], Float64]
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
        check_memory_layout_row_major(data, 1)
        if not isinstance(sampling_rate, int):
            raise BrainFlowError('wrong type for sampling rate', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance(filter_type, int):
            raise BrainFlowError('wrong type for filter type', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance().perform_lowpass(data, data.shape[0], sampling_rate, cutoff, order,
                                                            filter_type, ripple)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to perform low pass filter', res)

    @classmethod
    def perform_highpass(cls, data, sampling_rate: int, cutoff: float, order: int, filter_type: int,
                         ripple: float) -> None:
        """apply high pass filter to provided data

        :param data: data to filter, filter works in-place
        :type data: NDArray[Shape["*"], Float64]
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
        check_memory_layout_row_major(data, 1)
        if not isinstance(sampling_rate, int):
            raise BrainFlowError('wrong type for sampling rate', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance(filter_type, int):
            raise BrainFlowError('wrong type for filter type', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance().perform_highpass(data, data.shape[0], sampling_rate, cutoff, order,
                                                             filter_type, ripple)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to apply high pass filter', res)

    @classmethod
    def perform_bandpass(cls, data, sampling_rate: int, start_freq: float,
                         stop_freq: float, order: int, filter_type: int, ripple: float) -> None:
        """apply band pass filter to provided data

        :param data: data to filter, filter works in-place
        :type data: NDArray[Shape["*"], Float64]
        :param sampling_rate: board's sampling rate
        :type sampling_rate: int
        :param start_freq: start frequency
        :type start_freq: float
        :param stop_freq: stop frequency
        :type stop_freq: float
        :param order: filter order
        :type order: int
        :param filter_type: filter type from special enum
        :type filter_type: int
        :param ripple: ripple value for Chebyshev filter
        :type ripple: float
        """
        check_memory_layout_row_major(data, 1)
        if not isinstance(sampling_rate, int):
            raise BrainFlowError('wrong type for sampling rate', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance(filter_type, int):
            raise BrainFlowError('wrong type for filter type', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance().perform_bandpass(data, data.shape[0], sampling_rate, start_freq,
                                                             stop_freq, order, filter_type, ripple)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to apply band pass filter', res)

    @classmethod
    def perform_bandstop(cls, data, sampling_rate: int, start_freq: float,
                         stop_freq: float, order: int, filter_type: int, ripple: float) -> None:
        """apply band stop filter to provided data

        :param data: data to filter, filter works in-place
        :type data: NDArray[Shape["*"], Float64]
        :param sampling_rate: board's sampling rate
        :type sampling_rate: int
        :param start_freq: start frequency
        :type start_freq: float
        :param stop_freq: stop frequency
        :type stop_freq: float
        :param order: filter order
        :type order: int
        :param filter_type: filter type from special enum
        :type filter_type: int
        :param ripple: ripple value for Chebyshev filter
        :type ripple: float
        """
        check_memory_layout_row_major(data, 1)
        if not isinstance(sampling_rate, int):
            raise BrainFlowError('wrong type for sampling rate', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance(filter_type, int):
            raise BrainFlowError('wrong type for filter type', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance().perform_bandstop(data, data.shape[0], sampling_rate, start_freq,
                                                             stop_freq, order, filter_type, ripple)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to apply band stop filter', res)

    @classmethod
    def remove_environmental_noise(cls, data, sampling_rate: int, noise_type: float) -> None:
        """remove env noise using notch filter

        :param data: data to filter, filter works in-place
        :type data: NDArray[Shape["*"], Float64]
        :param sampling_rate: board's sampling rate
        :type sampling_rate: int
        :param noise_type: noise type
        :type noise_type: int
        """
        check_memory_layout_row_major(data, 1)
        if not isinstance(sampling_rate, int):
            raise BrainFlowError('wrong type for sampling rate', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance(noise_type, int):
            raise BrainFlowError('wrong type for noise type', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance().remove_environmental_noise(data, data.shape[0], sampling_rate, noise_type)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to apply notch filter', res)

    @classmethod
    def perform_rolling_filter(cls, data, period: int, operation: int) -> None:
        """smooth data using moving average or median

        :param data: data to smooth, it works in-place
        :type data: NDArray[Shape["*"], Float64]
        :param period: window size
        :type period: int
        :param operation: int value from AggOperation enum
        :type operation: int
        """
        check_memory_layout_row_major(data, 1)
        if not isinstance(period, int):
            raise BrainFlowError('wrong type for period', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance(operation, int):
            raise BrainFlowError('wrong type for operation', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        res = DataHandlerDLL.get_instance().perform_rolling_filter(data, data.shape[0], period, operation)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to smooth data', res)

    @classmethod
    def calc_stddev(cls, data):
        """calc stddev

        :param data: input array
        :type data: NDArray[Shape["*"], Float64]
        :return: stddev
        :rtype: float
        """
        check_memory_layout_row_major(data, 1)
        output = numpy.zeros(1).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().calc_stddev(data, 0, data.shape[0], output)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calc stddev', res)
        return output[0]

    @classmethod
    def get_railed_percentage(cls, data, gain: int):
        """get railed percentage

        :param data: input array
        :type data: NDArray[Shape["*"], Float64]
        :param gain: gain
        :type gain: int
        :return: railed percentage
        :rtype: float
        """
        check_memory_layout_row_major(data, 1)
        output = numpy.zeros(1).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().get_railed_percentage(data, data.shape[0], gain, output)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to get railed percentage', res)
        return output[0]

    @classmethod
    def get_oxygen_level(cls, ppg_ir, ppg_red, sampling_rate: int,
                         coef1=1.5958422, coef2=-34.6596622, coef3=112.6898759):
        """get oxygen level from ppg

        :param ppg_ir: input array
        :type ppg_ir: NDArray[Shape["*"], Float64]
        :param ppg_red: input array
        :type ppg_red: NDArray[Shape["*"], Float64]
        :param sampling_rate: sampling rate
        :type sampling_rate: int
        :return: oxygen level
        :rtype: float
        """
        check_memory_layout_row_major(ppg_ir, 1)
        check_memory_layout_row_major(ppg_red, 1)
        if ppg_ir.shape[0] != ppg_red.shape[0]:
            raise BrainFlowError('invalid shapes', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR)
        output = numpy.zeros(1).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().get_oxygen_level(ppg_ir, ppg_red, ppg_red.shape[0], sampling_rate,
                                                             coef1, coef2, coef3, output)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calc oxygen level', res)
        return output[0]

    @classmethod
    def get_heart_rate(cls, ppg_ir, ppg_red, sampling_rate: int, fft_size: int):
        """get heart rate

        :param ppg_ir: input array
        :type ppg_ir: NDArray[Shape["*"], Float64]
        :param ppg_red: input array
        :type ppg_red: NDArray[Shape["*"], Float64]
        :param sampling_rate: sampling rate
        :type sampling_rate: int
        :param fft_size: recommended 8192
        :type fft_size: int
        :return: heart rate
        :rtype: float
        """
        check_memory_layout_row_major(ppg_ir, 1)
        check_memory_layout_row_major(ppg_red, 1)
        if ppg_ir.shape[0] != ppg_red.shape[0]:
            raise BrainFlowError('invalid shapes', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR)
        output = numpy.zeros(1).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().get_heart_rate(ppg_ir, ppg_red, ppg_red.shape[0], sampling_rate,
                                                           fft_size, output)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calc heart rate', res)
        return output[0]

    @classmethod
    def perform_downsampling(cls, data, period: int, operation: int):
        """perform data downsampling, it doesnt apply lowpass filter for you, it just aggregates several data points

        :param data: initial data
        :type data: NDArray[Shape["*"], Float64]
        :param period: downsampling period
        :type period: int
        :param operation: int value from AggOperation enum
        :type operation: int
        :return: downsampled data
        :rtype: NDArray[Shape["*"], Float64]
        """
        check_memory_layout_row_major(data, 1)
        if not isinstance(period, int):
            raise BrainFlowError('wrong type for period', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not isinstance(operation, int):
            raise BrainFlowError('wrong type for operation', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if period <= 0:
            raise BrainFlowError('Invalid value for period', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)

        downsampled_data = numpy.zeros(int(data.shape[0] / period)).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().perform_downsampling(data, data.shape[0], period, operation,
                                                                 downsampled_data)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to perform downsampling', res)

        return downsampled_data

    @classmethod
    def perform_wavelet_transform(cls, data, wavelet: int, decomposition_level: int,
                                  extension_type=WaveletExtensionTypes.SYMMETRIC) -> Tuple:
        """perform wavelet transform

        :param data: initial data
        :type data: NDArray[Shape["*"], Float64]
        :param wavelet: use WaveletTypes enum
        :type wavelet: int
        :param decomposition_level: level of decomposition
        :type decomposition_level: int
        :param extension_type: extension type, use WaveletExtensionTypes
        :type externsion_type: int
        :return: tuple of wavelet coeffs in format [A(J) D(J) D(J-1) ..... D(1)] where J is decomposition level, A - app coeffs, D - detailed coeffs, and array with lengths for each block
        :rtype: tuple
        """
        check_memory_layout_row_major(data, 1)

        wavelet_coeffs = numpy.zeros(data.shape[0] + 2 * (40 + 1)).astype(numpy.float64)
        lengths = numpy.zeros(decomposition_level + 1).astype(numpy.int32)
        res = DataHandlerDLL.get_instance().perform_wavelet_transform(data, data.shape[0], wavelet,
                                                                      decomposition_level, extension_type,
                                                                      wavelet_coeffs, lengths)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to perform wavelet transform', res)

        return wavelet_coeffs[0: sum(lengths)], lengths

    @classmethod
    def restore_data_from_wavelet_detailed_coeffs(cls, data, wavelet, decomposition_level, level_to_restore):
        """restore data from a single wavelet coeff

        :param data: initial data
        :type data: NDArray[Shape["*"], Float64]
        :param wavelet: use WaveletTypes enum
        :type wavelet: int
        :param decomposition_level: level of decomposition
        :type decomposition_level: int
        :param level_to_restore: level of coeffs
        :type level_to_restore: int
        :return: 
        :rtype: NDArray[Shape["*"], Float64]
        """
        check_memory_layout_row_major(data, 1)

        output = numpy.zeros(data.shape[0])
        res = DataHandlerDLL.get_instance().restore_data_from_wavelet_detailed_coeffs(data, data.shape[0], wavelet,
                                                                 decomposition_level, level_to_restore, output)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to perfom restore_data_from_wavelet_detailed_coeffs', res)

        return output

    @classmethod
    def detect_peaks_z_score(cls, data, lag=5, threshold=3.5, influence=0.1):
        """z score algorithm for peak detection

        :param data: initial data
        :type data: NDArray[Shape["*"], Float64]
        :param lag: window size for averaging
        :type lag: int
        :param threshold: in stddev units
        :type threshold: float
        :param influence: contribution of peaks to mean value, between 0 and 1
        :type influence: float
        :return: 
        :rtype: NDArray[Shape["*"], Float64]
        """
        check_memory_layout_row_major(data, 1)

        output = numpy.zeros(data.shape[0])
        res = DataHandlerDLL.get_instance().detect_peaks_z_score(data, data.shape[0], lag,
                                                                 threshold, influence, output)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to perfom detect_peaks_z_score', res)

        return output

    @classmethod
    def perform_inverse_wavelet_transform(cls, wavelet_output: Tuple, original_data_len: int, wavelet: int,
                                          decomposition_level: int, extension_type=WaveletExtensionTypes.SYMMETRIC):
        """perform wavelet transform

        :param wavelet_output: tuple of wavelet_coeffs and array with lengths
        :type wavelet_coeffs: tuple
        :param original_data_len: len of signal before wavelet transform
        :type original_data_len: int
        :param wavelet: use WaveletTypes enum
        :type wavelet: int
        :param decomposition_level: level of decomposition
        :type decomposition_level: int
        :param extension_type: extension type, use WaveletExtensionTypes
        :type externsion_type: int
        :return: restored data
        :rtype: NDArray[Shape["*"], Float64]
        """
        original_data = numpy.zeros(original_data_len).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().perform_inverse_wavelet_transform(wavelet_output[0], original_data_len,
                                                                              wavelet,
                                                                              decomposition_level, extension_type,
                                                                              wavelet_output[1], original_data)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to perform inverse wavelet transform', res)

        return original_data

    @classmethod
    def perform_wavelet_denoising(cls, data, wavelet: int, decomposition_level: int,
                                  wavelet_denoising=WaveletDenoisingTypes.SURESHRINK,
                                  threshold=ThresholdTypes.HARD,
                                  extension_type=WaveletExtensionTypes.SYMMETRIC,
                                  noise_level=NoiseEstimationLevelTypes.FIRST_LEVEL) -> None:
        """perform wavelet denoising

        :param data: data to denoise
        :type data: NDArray[Shape["*"], Float64]
        :param wavelet: use WaveletTypes enum
        :type wavelet: int
        :param decomposition_level: decomposition level
        :type decomposition_level: int
        :param wavelet_denoising: use WaveletDenoisingTypes enum
        :type wavelet_denoising: int
        :param threshold: use ThresholdTypes enum
        :type threshold: int
        :param extension_type: use WaveletExtensionTypes enum
        :type extension_type: int
        :param noise_level: use NoiseEstimationLevelTypes enum
        :type noise_level: int
        """
        check_memory_layout_row_major(data, 1)

        res = DataHandlerDLL.get_instance().perform_wavelet_denoising(data, data.shape[0], wavelet,
                                                                      decomposition_level, wavelet_denoising, threshold,
                                                                      extension_type, noise_level)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to denoise data', res)

    @classmethod
    def get_csp(cls, data, labels) -> Tuple:
        """calculate filters and the corresponding eigenvalues using the Common Spatial Patterns

        :param data: [epochs x channels x times]-shaped 3D array of data for two classes
        :type data: NDArray[Shape["*, *, *"], Float64]
        :param labels: n_epochs-length 1D array of zeros and ones that assigns class labels for each epoch. Zero corresponds to the first class
        :type labels: NDArray[Shape["*"], Float64]
        :return: [channels x channels]-shaped 2D array of filters and [channels]-length 1D array of the corresponding eigenvalues
        :rtype: Tuple
        """
        if not (len(labels.shape) == 1):
            raise BrainFlowError('Invalid shape of array <labels>', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if not (len(labels) == data.shape[0]):
            raise BrainFlowError('Invalid number of elements in array <labels>',
                                 BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)

        n_epochs, n_channels, n_times = data.shape

        temp_data1d = numpy.reshape(data, (n_epochs * n_channels * n_times,))

        output_filters = numpy.zeros(int(n_channels * n_channels)).astype(numpy.float64)
        output_eigenvalues = numpy.zeros(int(n_channels)).astype(numpy.float64)

        res = DataHandlerDLL.get_instance().get_csp(temp_data1d, labels, n_epochs, n_channels, n_times, output_filters,
                                                    output_eigenvalues)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calc csp', res)

        output_filters = numpy.reshape(output_filters, (n_channels, n_channels))

        return output_filters, output_eigenvalues

    @classmethod
    def get_window(cls, window_function: int, window_len: int):
        """perform data windowing

        :param window_function: window function
        :type window: int
        :param window_len: len of the window function
        :return: numpy array, len of the array is the same as data
        :rtype: NDArray[Shape["*"], Float64]
        """
        window_data = numpy.zeros(int(window_len)).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().get_window(window_function, window_len, window_data)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to perform windowing', res)

        return window_data

    @classmethod
    def perform_fft(cls, data, window: int):
        """perform direct fft

        :param data: data for fft, len of data must be even
        :type data: NDArray[Shape["*"], Float64]
        :param window: window function
        :type window: int
        :return: numpy array of complex values, len of this array is N / 2 + 1
        :rtype: NDArray[Shape["*"], Complex128]
        """

        check_memory_layout_row_major(data, 1)

        temp_re = numpy.zeros(int(data.shape[0] / 2 + 1)).astype(numpy.float64)
        temp_im = numpy.zeros(int(data.shape[0] / 2 + 1)).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().perform_fft(data, data.shape[0], window, temp_re, temp_im)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to perform fft', res)

        output = numpy.zeros(int(data.shape[0] / 2 + 1)).astype(numpy.complex128)
        for i in range(output.shape[0]):
            output[i] = numpy.complex128(complex(temp_re[i], temp_im[i]))

        return output

    @classmethod
    def get_psd(cls, data, sampling_rate: int, window: int) -> Tuple:
        """calculate PSD

        :param data: data to calc psd, len of data must be even
        :type data: NDArray[Shape["*"], Float64]
        :param sampling_rate: sampling rate
        :type sampling_rate: int
        :param window: window function
        :type window: int
        :return: amplitude and frequency arrays of len N / 2 + 1
        :rtype: tuple
        """

        check_memory_layout_row_major(data, 1)

        ampls = numpy.zeros(int(data.shape[0] / 2 + 1)).astype(numpy.float64)
        freqs = numpy.zeros(int(data.shape[0] / 2 + 1)).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().get_psd(data, data.shape[0], sampling_rate, window, ampls, freqs)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calc psd', res)

        return ampls, freqs

    @classmethod
    def get_psd_welch(cls, data, nfft: int, overlap: int, sampling_rate: int, window: int) -> Tuple:
        """calculate PSD using Welch method

        :param data: data to calc psd
        :type data: NDArray[Shape["*"], Float64]
        :param nfft: FFT Window size, must be even
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

        check_memory_layout_row_major(data, 1)

        ampls = numpy.zeros(int(nfft / 2 + 1)).astype(numpy.float64)
        freqs = numpy.zeros(int(nfft / 2 + 1)).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().get_psd_welch(data, data.shape[0], nfft, overlap, sampling_rate, window,
                                                          ampls, freqs)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calc psd welch', res)

        return ampls, freqs

    @classmethod
    def detrend(cls, data, detrend_operation: int) -> None:
        """detrend data

        :param data: data to calc psd
        :type data: NDArray[Shape["*"], Float64]
        :param detrend_operation: Type of detrend operation
        :type detrend_operation: int
        """

        check_memory_layout_row_major(data, 1)
        res = DataHandlerDLL.get_instance().detrend(data, data.shape[0], detrend_operation)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to detrend data', res)

    @classmethod
    def get_band_power(cls, psd: Tuple, freq_start: float, freq_end: float) -> float:
        """calculate band power

        :param psd: psd from get_psd
        :type psd: tuple
        :param freq_start: start freq
        :type freq_start: int
        :param freq_end: end freq
        :type freq_end: int
        :return: band power
        :rtype: float
        """
        band_power = numpy.zeros(1).astype(numpy.float64)
        res = DataHandlerDLL.get_instance().get_band_power(psd[0], psd[1], psd[0].shape[0], freq_start, freq_end,
                                                           band_power)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calc band power', res)

        return band_power[0]

    @classmethod
    def get_avg_band_powers(cls, data, channels: List, sampling_rate: int, apply_filter: bool) -> Tuple:
        """calculate avg and stddev of BandPowers across all channels, bands are 1-4,4-8,8-13,13-30,30-50

        :param data: 2d array for calculation
        :type data: NDArray[Shape["*, *"], Float64]
        :param channels: channels - rows of data array which should be used for calculation
        :type channels: List
        :param sampling_rate: sampling rate
        :type sampling_rate: int
        :param apply_filter: apply bandpass and bandstop filtrers or not
        :type apply_filter: bool
        :return: avg and stddev arrays for bandpowers
        :rtype: tuple
        """

        bands = [(2.0, 4.0), (4.0, 8.0), (8.0, 13.0), (13.0, 30.0), (30.0, 45.0)]
        return cls.get_custom_band_powers(data, bands, channels, sampling_rate, apply_filter)

    @classmethod
    def get_custom_band_powers(cls, data, bands: List, channels: List, sampling_rate: int,
                               apply_filter: bool) -> Tuple:
        """calculate avg and stddev of BandPowers across selected channels

        :param data: 2d array for calculation
        :type data: NDArray[Shape["*, *"], Float64]
        :param bands: List of typles with bands to use. E.g [(1.5, 4.0), (4.0, 8.0), (8.0, 13.0), (13.0, 30.0), (30.0, 45.0)]
        :type bands: List
        :param channels: channels - rows of data array which should be used for calculation
        :type channels: List
        :param sampling_rate: sampling rate
        :type sampling_rate: int
        :param apply_filter: apply bandpass and bandstop filtrers or not
        :type apply_filter: bool
        :return: avg and stddev arrays for bandpowers
        :rtype: tuple
        """

        check_memory_layout_row_major(data, 2)
        if (len(channels) == 0) or (len(bands) == 0):
            raise BrainFlowError('wrong input for channels or bands', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        num_bands = len(bands)
        avg_bands = numpy.zeros(num_bands).astype(numpy.float64)
        stddev_bands = numpy.zeros(num_bands).astype(numpy.float64)
        data_1d = numpy.zeros(len(channels) * data.shape[1])
        start_freqs = numpy.zeros(num_bands)
        stop_freqs = numpy.zeros(num_bands)
        for i in range(num_bands):
            start_freqs[i] = bands[i][0]
            stop_freqs[i] = bands[i][1]
        for i, channel in enumerate(channels):
            for j in range(data.shape[1]):
                data_1d[j + data.shape[1] * i] = data[channel][j]
        res = DataHandlerDLL.get_instance().get_custom_band_powers(data_1d, len(channels), data.shape[1], start_freqs,
                                                                   stop_freqs, num_bands,
                                                                   sampling_rate, int(apply_filter), avg_bands,
                                                                   stddev_bands)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to get_avg_band_powers', res)

        return avg_bands, stddev_bands

    @classmethod
    def perform_ica(cls, data, num_components: int, channels=None) -> Tuple:
        """perform ICA

        :param data: 2d array for calculation
        :type data: NDArray[Shape["*, *"], Float64]
        :param num_components: number of components
        :type num_components: int
        :param channels: channels - rows of data array which should be used for calculation, if None use all
        :type channels: List
        :return: w, k, a, s matrixes as a tuple
        :rtype: tuple
        """
        check_memory_layout_row_major(data, 2)
        if len(data.shape) != 2:
            raise BrainFlowError('wrong number of dimensions', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        if num_components < 1:
            raise BrainFlowError('wrong number of components', BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)

        if not channels:
            channels_to_use = range(data.shape[0])
        else:
            channels_to_use = channels
    
        data_1d = numpy.zeros(len(channels_to_use) * data.shape[1]).astype(numpy.float64)

        w = numpy.zeros(num_components * num_components).astype(numpy.float64)
        k = numpy.zeros(len(channels_to_use) * num_components).astype(numpy.float64)
        a = numpy.zeros(num_components * len(channels_to_use)).astype(numpy.float64)
        s = numpy.zeros(data.shape[1] * num_components).astype(numpy.float64)

        for i, channel in enumerate(channels_to_use):
            for j in range(data.shape[1]):
                data_1d[j + data.shape[1] * i] = data[channel][j]

        res = DataHandlerDLL.get_instance().perform_ica(data_1d, len(channels_to_use), data.shape[1],
                                                        num_components, w, k, a, s)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calculate ICA', res)

        w = w.reshape(num_components, num_components)
        k = k.reshape(num_components, len(channels_to_use))
        a = a.reshape(len(channels_to_use), num_components)
        s = s.reshape(num_components, data.shape[1])

        return w, k, a, s

    @classmethod
    def perform_ifft(cls, data):
        """perform inverse fft

        :param data: data from fft
        :type data: NDArray[Shape["*"], Complex128]
        :return: restored data
        :rtype: NDArray[Shape["*"], Float64]
        """
        temp_re = numpy.zeros(data.shape[0]).astype(numpy.float64)
        temp_im = numpy.zeros(data.shape[0]).astype(numpy.float64)
        for i in range(data.shape[0]):
            temp_re[i] = data[i].real
            temp_im[i] = data[i].imag
        output = numpy.zeros(2 * (data.shape[0] - 1)).astype(numpy.float64)

        res = DataHandlerDLL.get_instance().perform_ifft(temp_re, temp_im, output.shape[0], output)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to perform ifft', res)

        return output

    @classmethod
    def get_nearest_power_of_two(cls, value: int) -> int:
        """calc nearest power of two

        :param value: input value
        :type value: int
        :return: nearest power of two
        :rtype: int
        """
        output = numpy.zeros(1).astype(numpy.int32)
        res = DataHandlerDLL.get_instance().get_nearest_power_of_two(value, output)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to calc nearest power of two', res)

        return output[0]

    @classmethod
    def write_file(cls, data, file_name: str, file_mode: str) -> None:
        """write data to file, in file data will be transposed

        :param data: data to store in a file
        :type data: NDArray[Shape["*, *"], Float64]
        :param file_name: file name to store data
        :type file_name: str
        :param file_mode: 'w' to rewrite file or 'a' to append data to file
        :type file_mode: str
        """

        check_memory_layout_row_major(data, 2)
        try:
            file = file_name.encode()
        except BaseException:
            file = file_name
        try:
            mode = file_mode.encode()
        except BaseException:
            mode = file_mode
        data_flatten = data.flatten()
        res = DataHandlerDLL.get_instance().write_file(data_flatten, data.shape[0], data.shape[1], file, mode)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to write file', res)

    @classmethod
    def read_file(cls, file_name: str):
        """read data from file

        :param file_name: file name to read
        :type file_name: str
        :return: 2d numpy array with data from this file, data will be transposed to original dimensions
        :rtype: NDArray[Shape["*, *"], Float64]
        """
        try:
            file = file_name.encode()
        except BaseException:
            file = file_name

        num_elements = numpy.zeros(1).astype(numpy.int32)
        res = DataHandlerDLL.get_instance().get_num_elements_in_file(file, num_elements)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to determine number of elements in file', res)

        data_arr = numpy.zeros(num_elements[0]).astype(numpy.float64)
        num_rows = numpy.zeros(1).astype(numpy.int32)
        num_cols = numpy.zeros(1).astype(numpy.int32)

        res = DataHandlerDLL.get_instance().read_file(data_arr, num_rows, num_cols, file, num_elements[0])
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to read file', res)

        if len(num_rows) == 0 or len(num_cols) == 0:
            return None

        data_arr = data_arr[0:num_rows[0] * num_cols[0]].reshape(num_rows[0], num_cols[0])
        return data_arr

    @classmethod
    def get_version(cls) -> str:
        """get version of brainflow libraries

        :return: version
        :rtype: str
        :raises BrainFlowError
        """
        string = numpy.zeros(64).astype(numpy.ubyte)
        string_len = numpy.zeros(1).astype(numpy.int32)
        res = DataHandlerDLL.get_instance().get_version_data_handler(string, string_len, 64)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to request info', res)
        return string.tobytes().decode('utf-8')[0:string_len[0]]

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
        res = DataHandlerDLL.get_instance().log_message_data_handler(log_level, msg)
        if res != BrainFlowExitCodes.STATUS_OK.value:
            raise BrainFlowError('unable to write log message', res)
