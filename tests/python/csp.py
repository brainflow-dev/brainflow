import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, WindowFunctions

from pathlib import Path


def main():
    # demo for calculating the CSP filters

    n_ep, n_ch, n_times = (2, 2, 4)
    labels = np.array([0, 1]).astype('float64') 
    
    data = np.array([[[6, 3, 1, 5], [3, 0, 5, 1]], [[1, 5, 6, 2], [5, 1, 2, 2]]]).astype('float64')
    
    filters, ev = DataFilter.get_csp(data, labels);
    # filters = 
    # -0.313406  0.0792153
    # -0.280803  -0.480046

if __name__ == "__main__":
    main()
