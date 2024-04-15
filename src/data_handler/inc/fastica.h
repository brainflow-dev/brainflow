#pragma once

#include <stdlib.h>
#include <vector>

#include "Eigen/Dense"
#include "brainflow_constants.h"


class FastICA
{

public:
    FastICA (int num_components, int max_it = 300, double tol = 0.0001)
    {
        this->max_it = max_it;
        this->num_components = num_components;
        this->tol = tol;
        alpha = 1;
        row_norm = false;
    }

    int compute (Eigen::MatrixXd &X);
    int get_matrixes (double *w_mat, double *k_mat, double *a_mat, double *s_mat);

private:
    Eigen::MatrixXd K;
    Eigen::MatrixXd W;
    Eigen::MatrixXd A;
    Eigen::MatrixXd S;

    Eigen::MatrixXd fast_ica_parallel_compute (const Eigen::MatrixXd &X);
    void scale (Eigen::Ref<Eigen::MatrixXd> m, bool, bool, bool ignore_invariants = false,
        std::vector<int> *zeros = NULL);
    void random_normal (Eigen::MatrixXd &m);

    int max_it;
    int num_components;
    double tol;
    int alpha;
    bool row_norm;
};
