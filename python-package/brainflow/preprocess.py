import os
import time
import numpy as np
import pandas as pd
import scipy
from scipy import signal
from scipy.fftpack import fft

from brainflow.board_shim import *

class DataHandler (object):

    def __init__ (self, board_id, numpy_data = None, csv_file = None):
        if board_id == CYTON.board_id:
            self.fs_hz = CYTON.fs_hz
            self.num_eeg_channels = CYTON.num_eeg_channels
        elif board_id == GANGLION.board_id:
            self.fs_hz = GANGLION.fs_hz
            self.num_eeg_channels = GANGLION.num_eeg_channels
        else:
            raise Exception ('Unsupported Board Type')

        if numpy_data is not None:
            columns = ['package_num']
            for i in range (self.num_eeg_channels):
                columns.append ('eeg%d' % (i + 1))
            for i in range (3):
                columns.append ('accel%d' % (i + 1))
            columns.append ('timestamp')
            self.eeg_df = pd.DataFrame (numpy_data, columns = columns, dtype = np.float64)
        elif csv_file is not None:
            self.eeg_df = pd.read_csv (csv_file)
        else:
            raise Exception ('no data prodvided')

    def save_csv (self, filename):
        if os.path.isfile (filename):
            self.eeg_df.to_csv (filename, mode = 'a', header = False, index = False)
        else:
            self.eeg_df.to_csv (filename, index = False)

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

    def notch_interference (self, notch_freq_hz = 50.0):
        res_df = pd.DataFrame ()
        bp_stop_hz = notch_freq_hz + 3.0 * np.array ([-1, 1])
        b, a = signal.butter (3, bp_stop_hz / (self.fs_hz / 2.0), 'bandstop')

        for column_name in self.eeg_df.columns:
            if column_name.startswith ('eeg'):
                res_df[column_name] = signal.lfilter (b, a, self.eeg_df[column_name], 0)
            else:
                res_df[column_name] = self.eeg_df[column_name]

        self.eeg_df = res_df

    def bandpass (self, order, start, stop):
        res_df = pd.DataFrame ()
        bp_hz = np.array ([start,stop])
        b, a = signal.butter (8, bp_hz / (self.fs_hz / 2.0), 'bandpass')

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

    def preprocess_data (self, order = 8, start = 1, stop = 50, calc_fft = False):
        self.remove_dc_offset ()
        self.notch_interference ()
        self.bandpass (order, start, stop)
        if calc_fft:
            self.calculate_fft ()
        return self.get_data ()
