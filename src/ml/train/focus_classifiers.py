import glob
import argparse
import statistics
import os
import time

import numpy as np

from sklearn import metrics
from sklearn.linear_model import LogisticRegression
from sklearn.dummy import DummyClassifier
from metric_learn import LMNN
from sklearn.neighbors import KNeighborsClassifier
from sklearn.decomposition import PCA
from sklearn.model_selection import train_test_split, cross_val_score

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations, WindowFunctions, DetrendOperations
from brainflow.ml_model import BrainFlowMetrics, BrainFlowClassifiers, MLModel, BrainFlowModelParams


def prepare_data ():
    # use different windows, its kinda data augmentation
    window_sizes = [4.0, 6.0, 8.0, 10.0]
    overlaps = [0.5, 0.45, 0.4, 0.35] # percentage of window_size
    dataset_x = list ()
    dataset_y = list ()
    for data_type in ('relaxed', 'focused'):
        for file in glob.glob (os.path.join ('data', data_type, '*', '*.csv')):
            print (file)
            board_id = os.path.basename (os.path.dirname (file))
            try:
                board_id = int (board_id)
                data = DataFilter.read_file (file)
                sampling_rate = BoardShim.get_sampling_rate (board_id)
                eeg_channels = get_eeg_channels(board_id)
                for num, window_size in enumerate (window_sizes):
                    if data_type == 'focused':
                        cur_pos = sampling_rate * 10 # skip a little more for focus
                    else:
                        cur_pos = sampling_rate * 3
                    while cur_pos + int (window_size * sampling_rate) < data.shape[1]:
                        data_in_window = data[:, cur_pos:cur_pos + int (window_size * sampling_rate)]
                        bands = DataFilter.get_avg_band_powers (data_in_window, eeg_channels, sampling_rate, True)
                        feature_vector = np.concatenate ((bands[0], bands[1]))
                        dataset_x.append (feature_vector)
                        if data_type == 'relaxed':
                            dataset_y.append (0)
                        else:
                            dataset_y.append (1)
                        cur_pos = cur_pos + int (window_size * overlaps[num] * sampling_rate)
            except Exception as e:
                print (str (e))

    print ('Class 1: %d Class 0: %d' % (len ([x for x in dataset_y if x == 1]), len ([x for x in dataset_y if x == 0])))

    return dataset_x, dataset_y

def get_eeg_channels(board_id):
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
    return eeg_channels

def train_regression (data):
    model = LogisticRegression (class_weight = 'balanced', solver = 'liblinear', max_iter = 3000)
    print('#### Logistic Regression ####')
    scores = cross_val_score (model, data[0], data[1], cv = 5, scoring = 'f1_macro', n_jobs = 8)
    print ('f1 macro %s' % str (scores))
    scores = cross_val_score (model, data[0], data[1], cv = 5, scoring = 'precision_macro', n_jobs = 8)
    print ('precision macro %s' % str (scores))
    scores = cross_val_score (model, data[0], data[1], cv = 5, scoring = 'recall_macro', n_jobs = 8)
    print ('recall macro %s' % str (scores))

    model.fit (data[0], data[1])
    print ("Logistic Regressition Coefficients:")
    print (model.intercept_, model.coef_)

def train_knn (data):
    model = KNeighborsClassifier (n_neighbors = 20)
    print('#### KNN ####')
    data_x = data[0].copy()
    for i, x in enumerate (data_x):
        for j in range (5, 10):
            data_x[i][j] = data_x[i][j] / 5 # idea to make stddev less important than avg, 5 random value
    scores = cross_val_score (model, data_x, data[1], cv = 5, scoring = 'f1_macro', n_jobs = 8)
    print ('f1 macro %s' % str (scores))
    scores = cross_val_score (model, data_x, data[1], cv = 5, scoring = 'precision_macro', n_jobs = 8)
    print ('precision macro %s' % str (scores))
    scores = cross_val_score (model, data_x, data[1], cv = 5, scoring = 'recall_macro', n_jobs = 8)
    print ('recall macro %s' % str (scores))

def write_dataset (data):
    # we prepare dataset in C++ code in compile time, need to generate header for it

    y_string = '%s' % (', '.join ([str (x) for x in data[1]]))
    x_string_tmp = ['{' + ', '.join ([str (y) for y in x]) + '}' for x in data[0]]
    x_string = '%s' % (',\n'.join (x_string_tmp))
    file_content = '''// generated file, dont change manually
#pragma once


// clang-format off

double brainflow_focus_x[%d][10] = {%s};
int brainflow_focus_y[%d] = {%s};

// clang-format on
''' % (len (data[0]), x_string, len (data[1]), y_string)

    file_path = os.path.join (os.path.dirname (os.path.realpath (__file__)), '..', 'inc', 'focus_dataset.h')
    with open(file_path, 'w') as f:
        f.write (file_content)

def test_brainflow_lr (data):
    print ('Test BrainFlow LR')
    params = BrainFlowModelParams (BrainFlowMetrics.CONCENTRATION.value, BrainFlowClassifiers.REGRESSION.value)
    model = MLModel (params)
    start_time = time.time ()
    model.prepare ()
    predicted = [model.predict (x) > 0.5 for x in data[0]]
    model.release ()
    stop_time = time.time ()
    print ('Total time %f' % (stop_time - start_time))
    print (confusion_matrix (data[1], predicted))

def test_brainflow_knn (data):
    print ('Test BrainFlow KNN')
    params = BrainFlowModelParams (BrainFlowMetrics.CONCENTRATION.value, BrainFlowClassifiers.KNN.value)
    model = MLModel (params)
    start_time = time.time ()
    model.prepare ()
    predicted = [model.predict (x) > 0.5 for x in data[0]]
    model.release ()
    stop_time = time.time ()
    print ('Total time %f' % (stop_time - start_time))
    print (confusion_matrix (data[1], predicted))

def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--test', action = 'store_true')
    args = parser.parse_args ()

    data = prepare_data ()
    if args.test:
        # since we port models from python to c++ we need to test it as well
        test_brainflow_knn (data)
        test_brainflow_lr (data)
    else:
        train_regression (data)
        train_knn (data)
        write_dataset (data)
    

if __name__ == '__main__':
    main ()
