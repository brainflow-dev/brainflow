import glob
import statistics
import os
import numpy as np

from sklearn import metrics
from sklearn.linear_model import LogisticRegression
from sklearn.dummy import DummyClassifier
from sklearn.model_selection import train_test_split, cross_val_score

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations, WindowFunctions, DetrendOperations
from brainflow.ml_model import BrainFlowMetrics, BrainFlowClassifiers, MLModel


def prepare_data ():
    window_size = 2.5
    overlap = 1.25
    dataset_x = list ()
    dataset_y = list ()
    algo_focused_scores = list ()
    algo_relaxed_scores = list ()
    for data_type in ('relaxed', 'focused'):
        for file in glob.glob (os.path.join ('data', data_type, '*', '*.csv')):
            print (file)
            board_id = os.path.basename (os.path.dirname (file))
            try:
                board_id = int (board_id)
                data = DataFilter.read_file (file)
                sampling_rate = BoardShim.get_sampling_rate (board_id)
                eeg_channels = BoardShim.get_eeg_channels (board_id)
                # optional: filter some channels we dont want to consider
                try:
                    eeg_names = BoardShim.get_eeg_names (board_id)
                    selected_channels = list ()
                    # blacklisted_channels = {'O1', 'O2'}
                    blacklisted_channels = set ()
                    for i, channel in enumerate (eeg_names):
                        if not channel in blacklisted_channels:
                            selected_channels.append (eeg_channels[i])
                    eeg_channels = selected_channels
                except Exception as e:
                    print (str (e))
                print ('channels to use: %s' % str (eeg_channels))
                cur_pos = sampling_rate * 3 # skip first 3 seconds of data
                while cur_pos + int (window_size * sampling_rate) < data.shape[1]:
                    data_in_window = data[:, cur_pos:cur_pos + int (window_size * sampling_rate)]
                    bands = DataFilter.get_avg_band_powers (data_in_window, eeg_channels, sampling_rate, True)
                    feature_vector = np.concatenate ((bands[0], bands[1]))
                    dataset_x.append (feature_vector)
                    if data_type == 'relaxed':
                        dataset_y.append (0)
                    else:
                        dataset_y.append (1)
                    cur_pos = cur_pos + int (overlap * sampling_rate)
            except Exception as e:
                print (str (e))

    print ('Class 1: %d Class 0: %d' % (len ([x for x in dataset_y if x == 1]), len ([x for x in dataset_y if x == 0])))

    return dataset_x, dataset_y

def train_regression (data):
    # print cross validation scores
    dummy_clf = DummyClassifier (strategy = 'stratified')
    scores = cross_val_score (dummy_clf, data[0], data[1], cv = 5, scoring = 'f1_macro', n_jobs = 4)
    print ('Dummy Scores:')
    print (scores)

    model = LogisticRegression ()
    scores = cross_val_score (model, data[0], data[1], cv = 5, scoring = 'f1_macro', n_jobs = 4)
    print (scores)

    model.fit (data[0], data[1])
    print ("Logistic Regressition Coefficients:")
    print (model.intercept_, model.coef_)


def main ():
    data = prepare_data ()
    train_regression (data)


if __name__ == '__main__':
    main ()
