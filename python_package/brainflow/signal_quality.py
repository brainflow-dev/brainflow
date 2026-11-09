"""Policies and native reports for opt-in, whole-channel-window quality checks."""
import ctypes
from collections import namedtuple
import enum

import numpy as np

from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes
from brainflow.utils import check_memory_layout_row_major


class SignalQualityFlags(enum.IntEnum):
    GOOD = 0
    NONFINITE = 1
    CLIPPED = 2
    FLATLINE = 4


class _NativeQualityParams(ctypes.Structure):
    _fields_ = [('struct_size', ctypes.c_int), ('version', ctypes.c_int),
                ('enable_clipping', ctypes.c_int), ('lower_bound', ctypes.c_double),
                ('upper_bound', ctypes.c_double), ('max_clipped_percentage', ctypes.c_double),
                ('flatline_tolerance', ctypes.c_double), ('max_flatline_seconds', ctypes.c_double),
                ('min_channels', ctypes.c_int)]


class _NativeChannelQuality(ctypes.Structure):
    _fields_ = [('flags', ctypes.c_int), ('nonfinite_percentage', ctypes.c_double),
                ('clipped_percentage', ctypes.c_double), ('flatline_percentage', ctypes.c_double),
                ('longest_flatline_seconds', ctypes.c_double)]


class SignalQualityOptions(namedtuple('SignalQualityOptionsBase',
        'clipping_bounds max_clipped_percentage flatline_tolerance max_flatline_seconds min_channels')):
    """Caller-selected thresholds, not a calibrated EEG artifact classifier.

    Clipping is disabled unless bounds in the input's units are supplied. Samples
    at either bound count as clipped; reject when the percentage is strictly above
    max_clipped_percentage. A flatline is a consecutive run of adjacent differences
    <= flatline_tolerance lasting >= max_flatline_seconds (0 disables rejection).
    Nonfinite samples always reject the channel. min_channels applies to band powers.
    """
    __slots__ = ()

    def __new__(cls, clipping_bounds=None, max_clipped_percentage=0.0,
                flatline_tolerance=0.0, max_flatline_seconds=0.5, min_channels=1):
        return super(SignalQualityOptions, cls).__new__(cls, clipping_bounds,
            max_clipped_percentage, flatline_tolerance, max_flatline_seconds, min_channels)

    def _native(self):
        try:
            if (isinstance(self.min_channels, (bool, np.bool_)) or
                    not isinstance(self.min_channels, (int, np.integer)) or
                    not 1 <= self.min_channels <= 2147483647):
                raise ValueError('min_channels must be a positive C int')
            bounds = (0.0, 0.0) if self.clipping_bounds is None else self.clipping_bounds
            if len(bounds) != 2:
                raise ValueError('clipping_bounds must contain two limits')
            return _NativeQualityParams(ctypes.sizeof(_NativeQualityParams), 1,
                                        int(self.clipping_bounds is not None), float(bounds[0]),
                                        float(bounds[1]), float(self.max_clipped_percentage),
                                        float(self.flatline_tolerance), float(self.max_flatline_seconds),
                                        int(self.min_channels))
        except (TypeError, ValueError, OverflowError) as error:
            raise BrainFlowError('invalid signal quality options: ' + str(error),
                                 BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value) from error


def _quality_input(data, channels, sampling_rate, options):
    check_memory_layout_row_major(data, 2)
    selected = np.asarray(channels)
    if (selected.ndim != 1 or selected.size == 0 or selected.dtype.kind not in 'iu' or
            np.any(selected < 0) or np.any(selected >= data.shape[0]) or
            np.unique(selected).size != selected.size or data.shape[1] < 2 or
            selected.size * data.shape[1] > 2147483647 or
            isinstance(sampling_rate, (bool, np.bool_)) or
            not isinstance(sampling_rate, (int, np.integer)) or
            not 1 <= sampling_rate <= 2147483647 or not isinstance(options, SignalQualityOptions)):
        raise BrainFlowError('invalid quality input, channels, sampling rate, or options',
                             BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
    gathered = np.ascontiguousarray(data[selected], dtype=np.float64).ravel()
    return selected, gathered, options._native(), (_NativeChannelQuality * len(selected))()


def _quality_result(selected, params, reports):
    quality = []
    for channel, report in zip(selected, reports):
        quality.append(dict(channel=int(channel), flags=report.flags,
                            reasons=[flag.name.lower() for flag in SignalQualityFlags
                                     if flag and report.flags & flag],
                            nonfinite_percentage=report.nonfinite_percentage,
                            clipped_percentage=report.clipped_percentage,
                            flatline_percentage=report.flatline_percentage,
                            longest_flatline_seconds=report.longest_flatline_seconds))
    return dict(channels=[int(ch) for ch in selected], quality=quality,
                used_channels=[q['channel'] for q in quality if not q['flags']],
                rejected_channels=[q['channel'] for q in quality if q['flags']],
                quality_policy=dict(version=1, clipping_bounds=(params.lower_bound, params.upper_bound)
                                    if params.enable_clipping else None,
                                    max_clipped_percentage=params.max_clipped_percentage,
                                    flatline_tolerance=params.flatline_tolerance,
                                    max_flatline_seconds=params.max_flatline_seconds,
                                    min_channels=params.min_channels))
