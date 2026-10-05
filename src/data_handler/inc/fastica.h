#pragma once

#include "Eigen/Dense"
#include "brainflow_constants.h"


class FastICA
{

public:
    FastICA (int num_components, int max_it = 1000, double tol = 0.0001, int seed = -1)
    {
        this->max_it = max_it;
        this->num_components = num_components;
        this->tol = tol;
        this->seed = seed;
    }

    int compute (Eigen::MatrixXd &X);
    int get_matrixes (double *w_mat, double *k_mat, double *a_mat, double *s_mat);

private:
    Eigen::MatrixXd K;
    Eigen::MatrixXd W;
    Eigen::MatrixXd A;
    Eigen::MatrixXd S;

    bool fast_ica_parallel_compute (const Eigen::MatrixXd &X, Eigen::MatrixXd &result);
    void random_normal (Eigen::MatrixXd &m);

    int max_it;
    int num_components;
    double tol;
    int seed;
};
