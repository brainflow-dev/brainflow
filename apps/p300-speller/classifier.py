import argparse
import os
import pandas as pd
import numpy as np
from scipy import signal
import logging
import yaml
import pickle
import sys

from sklearn.discriminant_analysis import LinearDiscriminantAnalysis
from sklearn.decomposition import PCA
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report
from sklearn.preprocessing import StandardScaler

from imblearn.over_sampling import SMOTE

from brainflow import *


def load_data (eeg_file, event_file):
    """Load data from files"""
    eeg_data = pd.read_csv (eeg_file)
    event_data = pd.read_csv (event_file)
    eeg_data.index.name = 'index'
    event_data.index.name = 'index'
    return eeg_data, event_data

def split_event_data_by_trials (event_data, settings):
    """Split entire events file by trials"""
    num_cols = settings['general']['num_cols']
    seq_per_trial = settings['training_params']['seq_per_trial']
    events_per_character = seq_per_trial * (num_cols + num_cols) # square matrix
    results = list ()

    for i in range (event_data.shape[0]//events_per_character):
        results.append (event_data.iloc[i * events_per_character: (i + 1) * events_per_character])
    return results

def get_data_from_event (eeg_data, event_timestamp, settings):
    """Get data associated with event"""
    first_point = event_timestamp + settings['general']['eeg_delay'] / 1000.0
    time_delta_ms = settings['general']['eeg_end'] - settings['general']['eeg_delay']
    if settings['general']['board'] == 'Cython':
        requested_data_points = int (CYTHON.fs_hz * time_delta_ms / 1000.0)
    else:
        raise ValueError ('invalid board')

    df = eeg_data[eeg_data['timestamp'] > first_point].groupby ('index', as_index = False).first ()
    if df.shape[0] < requested_data_points:
        return None
    df = df.iloc[0:requested_data_points]

    if settings['general']['board'] == 'Cython':
        data_handler = DataHandler (CYTHON.board_id, df.values)
        return data_handler.preprocess_data (order = 3, start = 0.1, stop = 50)
    else:
        raise ValueError ('invalid board')

def calculate_wavelet (eeg_data):
    """Calculate wavelet transform"""
    features = list ()
    widths = np.arange (1, 10, 2)
    for column_name in eeg_data.columns:
        features += signal.cwt (eeg_data[column_name], signal.ricker, widths = widths).flatten ().tolist ()
    return features

def prepare_data (eeg_data, event_data, settings, training = True):
    """Prepare data for classification"""
    data = list ()
    for index, event in event_data.iterrows ():
        if training:
            if (event['orientation'] == 'col' and (event['highlighted']) == event['trial_col'])\
            or (event['orientation'] == 'row' and (event['highlighted']) == event['trial_row']):
                target = 1
            else:
                target = 0
        event_eeg_data = get_data_from_event (eeg_data, event['event_start_time'], settings)
        if event_eeg_data is None:
            break
        event_eeg_data = event_eeg_data.select (lambda col: col.startswith ('eeg') and col not in ('eeg1', 'eeg2'), axis = 1)
        event_data = event_eeg_data.values.flatten ().tolist ()
        if training:
            event_data.append (target)
        data.append (event_data)

    data_df = pd.DataFrame (data)
    if training:
        data_x = data_df.iloc[:, 0:-1]
        data_y = data_df.iloc[:, -1]
        return data_x, data_y
    else:
        return data_df, None

def classify_lda (data_x, data_y, settings):
    """Train Classifier using LDA and RFE"""
    x_train, x_test, y_train, y_test = train_test_split (data_x, data_y, test_size = 0.2, random_state = 0, stratify = data_y)

    sm = SMOTE (sampling_strategy = 0.33, random_state = 2)
    x_train, y_train = sm.fit_resample (x_train, y_train)

    scaler = StandardScaler ()
    scaler.fit (x_train)
    x_train = scaler.transform (x_train)

    pca = PCA ()
    pca.fit (x_train, y_train)
    x_train = pca.transform (x_train)

    clf = LinearDiscriminantAnalysis ()
    clf.fit (x_train, y_train)

    x_test = scaler.transform (x_test)
    x_test = pca.transform (x_test)
    print (clf.score (x_test, y_test))
    predicted = clf.predict (x_test)
    print (classification_report (y_test, predicted))

    data_to_test = pca.transform (scaler.transform (data_x))
    scores = cross_val_score (clf, data_to_test, data_y, cv = 5, scoring = 'roc_auc', n_jobs = -1, verbose = 10)
    print ('ROC_AUC:', scores, np.mean (scores), np.std (scores))
    scores = cross_val_score (clf, data_to_test, data_y, cv = 5, scoring = 'balanced_accuracy', n_jobs = -1, verbose = 10)
    print ('Balanced Accuracy:', scores, np.mean (scores), np.std (scores))

    save = False
    if sys.version_info >= (3,0):
        command = input ('Use this classifier for production?[Y/n]:')
        if command != 'n':
            save = True
    else:
        command = raw_input ('Use this classifier for production?[Y/n]:')
        if command != 'n':
            save = True

    if save:
        print ('Saving Classifier and Transformers')
        with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data', settings['general']['scaler']), 'wb') as f:
            pickle.dump (scaler, f)
        with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data', settings['general']['classifier']), 'wb') as f:
            pickle.dump (clf, f)
        with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data', settings['general']['transformer']), 'wb') as f:
            pickle.dump (pca, f)

def get_decison (data_x, scaler, transformer, classifier):
    data_x = scaler.transform (data_x)
    data_x = transformer.transform (data_x)
    return classifier.decision_function (data_x)

def get_classes (data_x, scaler, transformer, classifier):
    data_x = scaler.transform (data_x)
    data_x = transformer.transform (data_x)
    return classifier.predict (data_x)

def test_fair (settings):
    with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data', settings['general']['scaler']), 'rb') as f:
        scaler = pickle.load (f)
    with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data', settings['general']['classifier']), 'rb') as f:
        classifier = pickle.load (f)
    with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data', settings['general']['transformer']), 'rb') as f:
        pca = pickle.load (f)

    eeg_data, event_data = load_data (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data','eeg_test.csv'),
                                    os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data','events_test.csv'))

    right_predictions = 0
    for i in range (settings['training_params']['num_trials']):
        start_index = i * settings['training_params']['seq_per_trial'] * settings['general']['num_cols'] * 2
        stop_index = (i + 1) * settings['training_params']['seq_per_trial'] * settings['general']['num_cols'] * 2
        current_event_data = event_data.iloc[start_index:stop_index]
        right_col = event_data['trial_col'].values[start_index]
        right_row = event_data['trial_row'].values[start_index]

        data_x, _ = prepare_data (eeg_data, current_event_data, settings)
        decisions = get_decison (data_x, scaler, pca, classifier)

        cols_weights = numpy.zeros (settings['general']['num_cols']).astype (numpy.float64)
        rows_weights = numpy.zeros (settings['general']['num_cols']).astype (numpy.float64)
        cols_predictions = numpy.zeros (settings['general']['num_cols']).astype (numpy.int64)
        rows_predictions = numpy.zeros (settings['general']['num_cols']).astype (numpy.int64)
        for i, decision in enumerate (decisions):
            event = current_event_data.iloc[i,:]
            if event['orientation'] == 'col':
                if decision > 0:
                    cols_predictions[event['highlighted']] = cols_predictions[event['highlighted']] + 1
                cols_weights[event['highlighted']] = cols_weights[event['highlighted']] + decision
            else:
                if decision > 0:
                    rows_predictions[event['highlighted']] = rows_predictions[event['highlighted']] + 1
                rows_weights[event['highlighted']] = rows_weights[event['highlighted']] + decision

        best_col_id = None
        best_row_id = None

        max_col_predictions = 0
        max_col_decision = 0
        max_row_predictions = 0
        max_row_decision = 0
        for i in range (settings['general']['num_cols']):
            if cols_predictions[i] > max_col_predictions:
                best_col_id = i
                max_col_predictions = cols_predictions[i]
                max_col_decision = cols_weights[i]
            elif cols_predictions[i] == max_col_predictions and max_col_decision < cols_weights[i]:
                best_col_id = i
                max_col_predictions = cols_predictions[i]
                max_col_decision = cols_weights[i]

            if rows_predictions[i] > max_row_predictions:
                best_row_id = i
                max_row_predictions = rows_predictions[i]
                max_row_decision = rows_weights[i]
            elif rows_predictions[i] == max_row_predictions and max_row_decision < rows_weights[i]:
                best_row_id = i
                max_row_predictions = rows_predictions[i]
                max_row_decision = rows_weights[i]


        logging.debug ('decision cols %s' % ' '.join ([str (x) for x in cols_predictions]))
        logging.debug ('decision rows %s' % ' '.join ([str (x) for x in rows_predictions]))
        logging.debug ('col: %s row:%s' % (str (best_col_id), str (best_row_id)))
        logging.debug ('right_col: %s right_row:%s' % (str (right_col), str (right_row)))

        if right_row == best_row_id and right_col == best_col_id:
            right_predictions = right_predictions + 1

    logging.debug ('Correct predictions %d' % right_predictions)


def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--eeg-file', type = str, help  = 'eeg file', default = os.path.join ('data', 'eeg.csv'))
    parser.add_argument ('--event-file', type = str, help  = 'event file', default = os.path.join ('data', 'events.csv'))
    parser.add_argument ('--settings', type = str, help  = 'settings file', default = 'ui_settings.yml')
    parser.add_argument ('--reuse', action = 'store_true')
    parser.add_argument ('--test', action = 'store_true')
    args = parser.parse_args ()
    logging.basicConfig (level = logging.DEBUG)

    settings = yaml.load (open (args.settings))
    if args.test:
        test_fair (settings)
    else:
        if args.reuse:
            with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data','data_x.pickle'), 'rb') as f:
                data_x = pickle.load (f)
            with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data','data_y.pickle'), 'rb') as f:
                data_y = pickle.load (f)
        else:
            eeg_data, event_data = load_data (args.eeg_file, args.event_file)
            data_x, data_y = prepare_data (eeg_data, event_data, settings)

            with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data','data_x.pickle'), 'wb') as f:
                pickle.dump (data_x, f)
            with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data','data_y.pickle'), 'wb') as f:
                pickle.dump (data_y, f)

        classify_lda (data_x, data_y, settings)


if __name__ == '__main__':
    main ()
