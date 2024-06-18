import glob
import argparse
import os
import pickle
import logging

import numpy as np
from sklearn.svm import SVC
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import RandomForestClassifier
from sklearn.ensemble import StackingClassifier
from sklearn.neighbors import KNeighborsClassifier
from sklearn.neural_network import MLPClassifier
from sklearn.model_selection import cross_val_score

from brainflow.board_shim import BoardShim
from brainflow.data_filter import DataFilter

from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType


def write_model(intercept, coefs, model_type):
    coefficients_string = '%s' % (','.join([str(x) for x in coefs[0]]))
    file_content = '''
#include "%s"
// clang-format off
const double %s_coefficients[%d] = {%s};
double %s_intercept = %lf;
// clang-format on
''' % (f'{model_type}_model.h', model_type, len(coefs[0]), coefficients_string, model_type, intercept)
    file_name = f'{model_type}_model.cpp'
    file_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', 'generated', file_name)
    with open(file_path, 'w') as f:
        f.write(file_content)

def prepare_data(first_class, second_class, blacklisted_channels=None):
    # use different windows, its kinda data augmentation
    window_sizes = [4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0]
    overlaps = [0.5, 0.475, 0.45, 0.425, 0.4, 0.375, 0.35] # percentage of window_size
    dataset_x = list()
    dataset_y = list()
    for data_type in (first_class, second_class):
        for file in glob.glob(os.path.join('data', data_type, '*', '*.csv')):
            data_x_temp = list()
            data_y_temp = list()
            logging.info(file)
            board_id = os.path.basename(os.path.dirname(file))
            try:
                board_id = int(board_id)
                data = DataFilter.read_file(file)
                sampling_rate = BoardShim.get_sampling_rate(board_id)
                eeg_channels = get_eeg_channels(board_id, blacklisted_channels)
                for num, window_size in enumerate(window_sizes):
                    cur_pos = sampling_rate * 10
                    while cur_pos + int(window_size * sampling_rate) < data.shape[1]:
                        data_in_window = data[:, cur_pos:cur_pos + int(window_size * sampling_rate)]
                        data_in_window = np.ascontiguousarray(data_in_window)
                        bands = DataFilter.get_avg_band_powers(data_in_window, eeg_channels, sampling_rate, True)
                        feature_vector = bands[0]
                        feature_vector = feature_vector.astype(float)
                        dataset_x.append(feature_vector)
                        data_x_temp.append(feature_vector)
                        if data_type == first_class:
                            dataset_y.append(0)
                            data_y_temp.append(0)
                        else:
                            dataset_y.append(1)
                            data_y_temp.append(0)
                        cur_pos = cur_pos + int(window_size * overlaps[num] * sampling_rate)
            except Exception as e:
                logging.error(str(e), exc_info=True)
            print_dataset_info((data_x_temp, data_y_temp))

    logging.info('1st Class: %d 2nd Class: %d' % (len([x for x in dataset_y if x == 0]), len([x for x in dataset_y if x == 1])))

    with open('dataset_x.pickle', 'wb') as f:
        pickle.dump(dataset_x, f, protocol=3)
    with open('dataset_y.pickle', 'wb') as f:
        pickle.dump(dataset_y, f, protocol=3)

    return dataset_x, dataset_y

def get_eeg_channels(board_id, blacklisted_channels):
    eeg_channels = BoardShim.get_eeg_channels(board_id)
    try:
        eeg_names = BoardShim.get_eeg_names(board_id)
        selected_channels = list()
        if blacklisted_channels is None:
            blacklisted_channels = set()
        for i, channel in enumerate(eeg_names):
            if not channel in blacklisted_channels:
                selected_channels.append(eeg_channels[i])
        eeg_channels = selected_channels
    except Exception as e:
        logging.warn(str(e))
    logging.info('channels to use: %s' % str(eeg_channels))
    return eeg_channels

def print_dataset_info(data):
    x, y = data
    first_class_ids = [idx[0] for idx in enumerate(y) if idx[1] == 0]
    second_class_ids = [idx[0] for idx in enumerate(y) if idx[1] == 1]
    x_first_class = list()
    x_second_class = list()
    
    for i, x_data in enumerate(x):
        if i in first_class_ids:
            x_first_class.append(x_data.tolist())
        elif i in second_class_ids:
            x_second_class.append(x_data.tolist())
    second_class_np = np.array(x_second_class)
    first_class_np = np.array(x_first_class)

    logging.info('1st Class Dataset Info:')
    logging.info('Mean:')
    logging.info(np.mean(first_class_np, axis=0))
    logging.info('2nd Class Dataset Info:')
    logging.info('Mean:')
    logging.info(np.mean(second_class_np, axis=0))

def train_regression_mindfulness(data):
    model = LogisticRegression(solver='liblinear', max_iter=4000,
                                penalty='l2', random_state=2, fit_intercept=False, intercept_scaling=3)
    logging.info('#### Logistic Regression ####')
    scores = cross_val_score(model, data[0], data[1], cv=5, scoring='f1_macro', n_jobs=8)
    logging.info('f1 macro %s' % str(scores))
    model.fit(data[0], data[1])
    logging.info(model.intercept_)
    logging.info(model.coef_)
    
    initial_type = [('mindfulness_input', FloatTensorType([1, 5]))]
    onx = convert_sklearn(model, initial_types=initial_type, target_opset=11, options={type(model): {'zipmap': False}})
    with open('logreg_mindfulness.onnx', 'wb') as f:
        f.write(onx.SerializeToString())
    write_model(model.intercept_, model.coef_, 'mindfulness')

def train_svm_mindfulness(data):
    model = SVC(kernel='linear', verbose=True, random_state=1, class_weight='balanced', probability=True)
    logging.info('#### SVM ####')
    model.fit(data[0], data[1])
    initial_type = [('mindfulness_input', FloatTensorType([1, 5]))]
    onx = convert_sklearn(model, initial_types=initial_type, target_opset=11, options={type(model): {'zipmap': False}})
    with open('svm_mindfulness.onnx', 'wb') as f:
        f.write(onx.SerializeToString())

def train_random_forest_mindfulness(data):
    model = RandomForestClassifier(class_weight='balanced', random_state=1, n_jobs=15, n_estimators=200)
    logging.info('#### Random Forest ####')
    scores = cross_val_score(model, data[0], data[1], cv=5, scoring='f1_macro', n_jobs=15)
    logging.info('f1 macro %s' % str(scores))
    model.fit(data[0], data[1])

    initial_type = [('mindfulness_input', FloatTensorType([1, 5]))]
    onx = convert_sklearn(model, initial_types=initial_type, target_opset=11, options={type(model): {'zipmap': False}})
    with open('forest_mindfulness.onnx', 'wb') as f:
        f.write(onx.SerializeToString())

def train_knn_mindfulness(data):
    model = KNeighborsClassifier(n_neighbors=10, n_jobs=8)
    logging.info('#### KNN ####')
    scores = cross_val_score(model, data[0], data[1], cv=5, scoring='f1_macro', n_jobs=15)
    logging.info('f1 macro %s' % str(scores))
    model.fit(data[0], data[1])

    initial_type = [('mindfulness_input', FloatTensorType([1, 5]))]
    onx = convert_sklearn(model, initial_types=initial_type, target_opset=11, options={type(model): {'zipmap': False}})
    with open('knn_mindfulness.onnx', 'wb') as f:
        f.write(onx.SerializeToString())

def train_mlp_mindfulness(data):
    model = MLPClassifier(hidden_layer_sizes=(100, 20),learning_rate='adaptive', max_iter=1000,
                          random_state=1, verbose=True, activation='logistic', solver='adam')
    logging.info('#### MLP ####')
    scores = cross_val_score(model, data[0], data[1], cv=5, scoring='f1_macro', n_jobs=15)
    logging.info('f1 macro %s' % str(scores))
    model.fit(data[0], data[1])

    initial_type = [('mindfulness_input', FloatTensorType([1, 5]))]
    onx = convert_sklearn(model, initial_types=initial_type, target_opset=11, options={type(model): {'zipmap': False}})
    with open('mlp_mindfulness.onnx', 'wb') as f:
        f.write(onx.SerializeToString())

def train_stacking_classifier(data):
    model1 = MLPClassifier(hidden_layer_sizes=(100, 20),learning_rate='adaptive', max_iter=1000,
                          random_state=1, verbose=True, activation='logistic', solver='adam')
    model2 = KNeighborsClassifier(n_neighbors=10, n_jobs=8)
    model3 = RandomForestClassifier(class_weight='balanced', random_state=1, n_jobs=8, n_estimators=200)
    meta_model = LogisticRegression()
    sclf = StackingClassifier(estimators=[('MLPClassifier', model1), ('KNeighborsClassifier', model2), ('RandomForestClassifier', model3)],
                              final_estimator=meta_model, n_jobs=15,
                              passthrough=True)
    logging.info('#### Stacking ####')
    scores = cross_val_score(sclf, data[0], data[1], cv=5, scoring='f1_macro', n_jobs=15)
    logging.info('f1 macro %s' % str(scores))
    sclf.fit(data[0], data[1])

    initial_type = [('mindfulness_input', FloatTensorType([1, 5]))]
    onx = convert_sklearn(sclf, initial_types=initial_type, target_opset=11, options={type(sclf): {'zipmap': False}})
    with open('stacking_mindfulness.onnx', 'wb') as f:
        f.write(onx.SerializeToString())

def main():
    logging.basicConfig(level=logging.INFO)
    parser = argparse.ArgumentParser()
    parser.add_argument('--reuse-dataset', action='store_true')
    args = parser.parse_args()

    if args.reuse_dataset:
        with open('dataset_x.pickle', 'rb') as f:
            dataset_x = pickle.load(f)
        with open('dataset_y.pickle', 'rb') as f:
            dataset_y = pickle.load(f)
        data = dataset_x, dataset_y
    else:
        data = prepare_data('relaxed', 'focused')
    print_dataset_info(data)
    train_regression_mindfulness(data)
    train_svm_mindfulness(data)
    train_knn_mindfulness(data)
    train_random_forest_mindfulness(data)
    train_mlp_mindfulness(data)
    train_stacking_classifier(data)


if __name__ == '__main__':
    main()
