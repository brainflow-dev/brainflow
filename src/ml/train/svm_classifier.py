import pickle
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report
from libsvm.svmutil import *
import multiprocessing as mp
import argparse
import time

def split_data(x_list,dataset_y):
    x_train, x_test, y_train, y_test = train_test_split(x_list, dataset_y, test_size=0.2, random_state=1)
    x_train, x_vald, y_train, y_vald = train_test_split(x_train, y_train, test_size=0.25, random_state=1)
    return (x_train, y_train, x_vald, y_vald, x_test, y_test)

def train_model(C, gamma, x_train, y_train, x_vald, y_vald):
    prob = svm_problem (y_train, x_train)
    par_str = f'-c {C} -g {gamma}  -b 1'
    param = svm_parameter (par_str)
    m = svm_train (prob, param)
    p_label, p_acc, p_val = svm_predict (y_vald, x_vald, m)
    metrics =  classification_report (y_vald,p_label,output_dict=True)
    return (metrics,C,gamma) 

def train_brainflow_search_svm(x_train, y_train, x_vald, y_vald):
    print("Train Brainflow Grid Search")
    C = [2 ** x for x in range(-4, 16)]
    gamma = [2 ** i for i in range(-16, 4)]
    pool = mp.Pool (mp.cpu_count())
    M = pool.starmap_async (train_model, [(c, y, x_train, y_train, x_vald, y_vald) for c, y in zip (C, gamma)])
    M = M.get ()
    max_f1_score, max_C, max_Gamma = 0, 0, 0
    max_metrics = {}
    pool.close ()
    for metrics,C,gamma in M:
        if metrics["weighted avg"]["f1-score"] > max_f1_score:
            max_f1_score = metrics["weighted avg"]["f1-score"]
            max_C = C
            max_metrics = metrics
            max_Gamma = gamma
    prob = svm_problem (y_train, x_train)
    par_str = f'-c {max_C} -g {max_Gamma} -b 1 '
    param = svm_parameter (par_str)
    model = svm_train (prob, param)
    print('#### SVM  ####')
    print(f'Optimal C:{max_C}.\n Optimal gamma:{max_Gamma}.')
    print (metrics)
    svm_save_model ('brainflow_svm.model', model)
    return model

def train_brainflow_svm(x_train, y_train, x_vald, y_vald):
    C = 256
    gamma = 4
    print('#### SVM  ####')
    prob = svm_problem (y_train, x_train)
    par_str = f'-c {C} -g {gamma} -b 1 '
    param = svm_parameter (par_str)
    model = svm_train(prob, param)
    p_label, p_acc, p_val = svm_predict (y_vald, x_vald, model)
    metrics = classification_report(y_vald, p_label)
    print(metrics)
    svm_save_model ('brainflow_svm.model', model)

def test_brainflow_model_svm(y_test, x_test):
    print('Test BrainFlow SVM')
    start_time = time.time ()
    model = svm_load_model('brainflow_svm.model')
    p_label, p_acc, p_val = svm_predict(y_test, x_test, model, '-b 1')
    predicted = [x[1] >= 0.5 for x in p_val]
    stop_time = time.time ()
    print('Total time %f' % (stop_time - start_time))
    metrics =  classification_report (y_test,p_label)
    print(f'Test Model Metrics :{metrics}')





    

    
