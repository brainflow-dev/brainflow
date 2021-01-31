import os


def write_knn_model(data):
    # we prepare dataset in C++ code in compile time, need to generate header for it

    y_string = '%s' % (', '.join([str(x) for x in data[1]]))
    x_string_tmp = ['{' + ', '.join([str(y) for y in x]) + '}' for x in data[0]]
    x_string = '%s' % (',\n'.join(x_string_tmp))
    file_content = '''
#include "focus_dataset.h"

// clang-format off

double brainflow_focus_x[%d][10] = {%s};
int brainflow_focus_y[%d] = {%s};

// clang-format on
''' % (len(data[1]), x_string, len(data[1]), y_string)

    file_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', 'generated', 'focus_dataset.cpp')
    with open(file_path, 'w') as f:
        f.write(file_content)

    header_content = '''
#pragma once

extern double brainflow_focus_x[%d][10];
extern int brainflow_focus_y[%d];
''' % (len(data[1]), len(data[1]))

    file_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', 'inc', 'focus_dataset.h')
    with open(file_path, 'w') as f:
        f.write(header_content)


def write_model(intercept, coefs, model_type):
    # we prepare dataset in C++ code in compile time, need to generate header for it
    coefficients_string = '%s' % (','.join([str(x) for x in coefs[0]]))
    file_content = '''
#include "%s"

// clang-format off

const double %s_coefficients[10] = {%s};
double %s_intercept = %lf;

// clang-format on
''' % (f'{model_type}_model.h', model_type, coefficients_string, model_type, intercept)
    file_name = f'{model_type}_model.cpp'
    file_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', 'generated', file_name)
    with open(file_path, 'w') as f:
        f.write(file_content)
