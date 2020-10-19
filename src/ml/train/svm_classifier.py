import pickle
from sklearn.model_selection import train_test_split
from libsvm.svmutil import *
import multiprocessing as mp
import argparse
import time

def create_dataset():
    with open ('dataset_x.pickle', 'rb') as f:
        dataset_x = pickle.load (f)
    with open ('dataset_y.pickle', 'rb') as f:
        dataset_y = pickle.load(f)
    xlist = list()
    for i in dataset_x:
        xlist.append(i.tolist())
    return (xlist, dataset_y)

def split_data(x_list,dataset_y):
    x_train, x_test, y_train, y_test = train_test_split(x_list, dataset_y, test_size=0.2, random_state=1)
    x_train, x_vald, y_train, y_vald = train_test_split(x_train, y_train, test_size=0.25, random_state=1)
    return (x_train, y_train, x_vald, y_vald, x_test, y_test)

def train_model(C, gamma, x_train, y_train, x_vald, y_vald):
    prob = svm_problem(y_train, x_train)
    par_str = f'-c {C} -g {gamma}'
    param = svm_parameter(par_str)
    m = svm_train(prob, param)
    p_label, p_acc, p_val = svm_predict(y_vald, x_vald, m)
    return (p_acc[0],m) 

def train_svm_grid_search(x_train, y_train, x_vald, y_vald):
    C = [2 ** x for x in range(-6, 16)]
    gamma = [2 ** i for i in range(-16, 4)]
    pool = mp.Pool(mp.cpu_count())
    M = pool.starmap(train_model, [(c, y, x_train, y_train, x_vald, y_vald) for c, y in zip(C, gamma)])
    max_acc = 0
    model = None
    for acc,m in M:
        if acc > max_acc:
            max_acc = acc
            model = m
    svm_save_model('brainflow_svm.model', model)
    return model
'''
TODO: Create a method to get the F1,precision,recall metrics currently using accuracy.
'''
def test_model(y_test, x_test):
    print('Test BrainFlow SVM')
    start_time = time.time ()
    model = svm_load_model('brainflow_svm.model')
    p_label, p_acc, p_val = svm_predict(y_test, x_test, model)
    stop_time = time.time ()
    print ('Total time %f' % (stop_time - start_time))
    print(f'Test Model Accuracy :{p_acc[0]}')

def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--test', action = 'store_true')
    args = parser.parse_args ()
    (xlist, dataset_y) = create_dataset()
    (x_train, y_train, x_vald, y_vald, x_test, y_test) = split_data(xlist, dataset_y)
    if args.test:
        # since we port models from python to c++ we need to test it as well
        test_model(y_test, x_test)
    else:
        train_svm_grid_search(x_train, y_train, x_vald, y_vald)

if __name__ == '__main__':
    main ()
    





    

    
