import time
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use ('Agg')
import matplotlib.pyplot as plt 

from brainflow import *

float_formatter = lambda x: "%.5f" % x
numpy.set_printoptions (formatter={'float_kind': float_formatter}, threshold=numpy.inf)

class CythonDataHandler (object):

    def __init__ (self, numpy_data):
        self.numpy_data = numpy_data

        columns = ['package_num']
        for i in range (8):
            columns.append ('eeg%d' % (i + 1))
        for i in range (3):
            columns.append ('accel%d' % (i + 1))
        columns.append ('timestamp')

        self.pandas_df = pd.DataFrame (self.numpy_data, columns = columns, dtype = np.float64)

    def draw_data (self, filename):
        eeg_df = self.pandas_df[['eeg1', 'eeg2', 'eeg3', 'eeg4', 'eeg5', 'eeg6', 'eeg7', 'eeg8', 'timestamp']]

        eeg_df.plot (x = 'timestamp')
        fig = plt.gcf ()
        fig.savefig (filename)


def main ():
    board = CythonBoard ('/dev/ttyUSB0')
    board.prepare_session ()
    board.start_stream ()
    time.sleep (5)
    # get last package
    print (board.get_immediate_board_data ())
    # get last 250 datapoints
    current_data = board.get_current_board_data (250)
    # get all data
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    data_handler = CythonDataHandler (data)
    print (data_handler.pandas_df)
    data_handler.draw_data ('data.png')

    with open ('current_data.raw', 'wt') as f:
        f.write (str (current_data))

    with open ('all_data.raw', 'wt') as f:
        f.write (str (data))


if __name__ == "__main__":
    main ()