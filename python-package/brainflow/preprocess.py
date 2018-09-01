import time
import numpy as np
import pandas as pd
import argparse
import scipy
from scipy import signal
from scipy.fftpack import fft

from brainflow.board_shim import *

class DataHandler (object):

    def __init__ (self, board_type, numpy_data):
        if board_type == Boards.Cython.value:
            self.fs_hz = 250
            self.num_eeg_channels = 8
        else:
            raise Exception ('Unsupported Board Type')
        self.numpy_data = numpy_data

        columns = ['package_num']
        for i in range (self.num_eeg_channels):
            columns.append ('eeg%d' % (i + 1))
        for i in range (3):
            columns.append ('accel%d' % (i + 1))
        columns.append ('timestamp')
        self.eeg_df = pd.DataFrame (self.numpy_data, columns = columns, dtype = np.float64)

    def remove_dc_offset (self):
        res_df = pd.DataFrame ()
        hp_cutoff_hz = 1.0
        b, a = signal.butter (2, hp_cutoff_hz / (self.fs_hz / 2.0), 'highpass')

        for column_name in self.eeg_df.columns:
            if column_name.startswith ('eeg'):
                res_df[column_name] = signal.lfilter (b, a, self.eeg_df[column_name], 0)
            else:
                res_df[column_name] = self.eeg_df[column_name]

        self.eeg_df = res_df

    def notch_interference (self):
        res_df = pd.DataFrame ()
        notch_freq_hz = np.array ([50.0, 60.0])
        for freq_hz in np.nditer (notch_freq_hz):
            bp_stop_hz = freq_hz + 3.0 * np.array ([-1, 1])
            b, a = signal.butter (3, bp_stop_hz / (self.fs_hz / 2.0), 'bandstop')

            for column_name in self.eeg_df.columns:
                if column_name.startswith ('eeg'):
                    res_df[column_name] = signal.lfilter (b, a, self.eeg_df[column_name], 0)
                else:
                    res_df[column_name] = self.eeg_df[column_name]

        self.eeg_df = res_df

    def bandpass (self, start, stop):
        res_df = pd.DataFrame ()
        bp_hz = np.array ([start,stop])
        b, a = signal.butter (3, bp_hz / (self.fs_hz / 2.0), 'bandpass')

        for column_name in self.eeg_df.columns:
            if column_name.startswith ('eeg'):
                res_df[column_name] = signal.lfilter (b, a, self.eeg_df[column_name], 0)
            else:
                res_df[column_name] = self.eeg_df[column_name]

        self.eeg_df = res_df

    def calculate_fft (self):
        res_df = pd.DataFrame ()

        for column_name in self.eeg_df.columns:
            if column_name.startswith ('eeg'):
                res_df['fft_' + column_name] = fft (self.eeg_df[column_name])
            res_df[column_name] = self.eeg_df[column_name]

        self.eeg_df = res_df

    def get_data (self):
        return self.eeg_df

    def preprocess_data (self, start = 1, stop = 50, calc_fft = False):
        self.remove_dc_offset ()
        self.notch_interference ()
        self.bandpass (start, stop)
        if calc_fft:
            self.calculate_fft ()
        return self.get_data ()
