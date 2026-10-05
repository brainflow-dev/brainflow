#include "fastica.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>

namespace
{
    bool decorrelate (Eigen::MatrixXd &matrix)
    {
        if (!matrix.allFinite ())
        {
            return false;
        }
        Eigen::JacobiSVD<Eigen::MatrixXd> svd (matrix, Eigen::ComputeFullU | Eigen::ComputeFullV);
        const Eigen::VectorXd &values = svd.singularValues ();
        if (!values.allFinite () || values[0] <= 0.0 ||
            values[values.size () - 1] <=
                values[0] * std::numeric_limits<double>::epsilon () * matrix.rows ())
        {
            return false;
        }
        // The polar factor is symmetric decorrelation without dividing by small
        // singular values or explicitly forming an inverse.
        matrix = svd.matrixU () * svd.matrixV ().transpose ();
        return matrix.allFinite ();
    }
}

int FastICA::compute (Eigen::MatrixXd &X)
{
    int rows = (int)X.rows ();
    int cols = (int)X.cols ();
    if ((num_components < 2) || (max_it < 1) || (rows < 2) || (cols < 3) ||
        (num_components > std::min (rows, cols - 1)) || !std::isfinite (tol) || (tol <= 0.0) ||
        (tol >= 1.0) || (seed < -1) || !X.allFinite ())
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // Work at unit scale so centering and covariance do not overflow for otherwise
    // representable signals. K and A below retain their original physical units.
    double input_scale = X.cwiseAbs ().maxCoeff ();
    if (!(input_scale > 0.0))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    X /= input_scale;
    Eigen::VectorXd means = X.rowwise ().mean ();
    X.colwise () -= means;
    Eigen::MatrixXd covariance = X * (X / (double)cols).transpose ();
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver (covariance);
    if (solver.info () != Eigen::Success || !solver.eigenvalues ().allFinite ())
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    double largest = solver.eigenvalues ()[rows - 1];
    double rank_tolerance =
        largest * std::numeric_limits<double>::epsilon () * std::max (rows, cols);
    if (!(largest > 0.0) || solver.eigenvalues ()[rows - num_components] <= rank_tolerance)
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    Eigen::MatrixXd basis (rows, num_components);
    Eigen::VectorXd scales (num_components);
    for (int i = 0; i < num_components; i++)
    {
        basis.col (i) = solver.eigenvectors ().col (rows - 1 - i);
        scales[i] = std::sqrt (solver.eigenvalues ()[rows - 1 - i]);
    }
    Eigen::MatrixXd whitening = scales.cwiseInverse ().asDiagonal () * basis.transpose ();
    Eigen::MatrixXd whitened = whitening * X;
    Eigen::MatrixXd weights;
    if (!whitened.allFinite () || !fast_ica_parallel_compute (whitened, weights))
    {
        // A nonconverged estimate must not be silently returned as successful ICA.
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    W = weights;
    K = whitening / input_scale;
    S = weights * whitened;
    // W is orthogonal, so this is pinv(W*K), without an unstable matrix inverse.
    // Keep A channels-by-components, matching every documented output shape.
    A = (basis * scales.asDiagonal () * weights.transpose ()) * input_scale;
    if (!W.allFinite () || !K.allFinite () || !S.allFinite () || !A.allFinite ())
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

bool FastICA::fast_ica_parallel_compute (const Eigen::MatrixXd &X, Eigen::MatrixXd &result)
{
    Eigen::MatrixXd weights (num_components, num_components);
    random_normal (weights);
    if (!decorrelate (weights))
    {
        return false;
    }

    const double samples = (double)X.cols ();
    for (int iteration = 0; iteration < max_it; iteration++)
    {
        Eigen::MatrixXd activation = (weights * X).array ().tanh ().matrix ();
        Eigen::VectorXd derivative = (1.0 - activation.array ().square ()).rowwise ().mean ();
        Eigen::MatrixXd next =
            activation * (X / samples).transpose () - derivative.asDiagonal () * weights;
        if (!decorrelate (next))
        {
            return false;
        }
        double change =
            ((next * weights.transpose ()).diagonal ().array ().abs () - 1.0).abs ().maxCoeff ();
        weights.swap (next);
        if (change < tol)
        {
            result = weights;
            return true;
        }
    }
    return false;
}

void FastICA::random_normal (Eigen::MatrixXd &matrix)
{
    std::mt19937 generator;
    if (seed < 0)
    {
        std::random_device device;
        generator.seed (device ());
    }
    else
    {
        generator.seed ((unsigned int)seed);
    }
    std::normal_distribution<double> distribution (0.0, 1.0);
    for (int row = 0; row < matrix.rows (); row++)
    {
        for (int col = 0; col < matrix.cols (); col++)
        {
            matrix (row, col) = distribution (generator);
        }
    }
}

int FastICA::get_matrixes (double *w_mat, double *k_mat, double *a_mat, double *s_mat)
{
    if (!w_mat || !k_mat || !a_mat || !s_mat || W.size () == 0 || K.size () == 0 ||
        A.size () == 0 || S.size () == 0)
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMatrix;
    Eigen::Map<RowMatrix> w_output (w_mat, W.rows (), W.cols ());
    Eigen::Map<RowMatrix> k_output (k_mat, K.rows (), K.cols ());
    Eigen::Map<RowMatrix> a_output (a_mat, A.rows (), A.cols ());
    Eigen::Map<RowMatrix> s_output (s_mat, S.rows (), S.cols ());
    w_output = W;
    k_output = K;
    a_output = A;
    s_output = S;
    return (int)BrainFlowExitCodes::STATUS_OK;
}
