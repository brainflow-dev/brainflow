#include "fastica.h"

#include <cmath>
#include <random>

// https://en.wikipedia.org/wiki/FastICA
// https://arnauddelorme.com/ica_for_dummies/
int FastICA::compute (Eigen::MatrixXd &X)
{
    int rows = (int)X.rows ();
    int cols = (int)X.cols ();
    int min_rows_cols = rows < cols ? rows : cols;
    if ((num_components < 2) || (max_it < 1) || (rows < 2) || (cols < 2) ||
        (num_components > min_rows_cols))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    W.resize (num_components, num_components);
    A.resize (rows, num_components);
    K.resize (num_components, rows);
    S.resize (num_components, cols);

    scale (X, true, row_norm);

    // Whitening
    // X %*% t(X)/rows
    Eigen::MatrixXd V = X * (X.array () / cols).matrix ().transpose ();
    // s <- La.svd(V)
    Eigen::BDCSVD<Eigen::MatrixXd> s (V, Eigen::ComputeThinU | Eigen::ComputeThinV);
    // D <- diag(c(1/sqrt(s$d)))
    Eigen::MatrixXd D = s.singularValues ().array ().sqrt ().inverse ().matrix ().asDiagonal ();
    // K <- D %*% t(s$u)
    Eigen::MatrixXd K_temp = D * s.matrixU ().transpose ();
    // K <- matrix( K[1:rows.comp, ], rows.comp, cols)
    Eigen::MatrixXd K_temp2 = K_temp.block (0, 0, num_components, rows);
    // X1 <- K %*% X
    Eigen::MatrixXd X1 = K_temp2 * X;
    Eigen::MatrixXd a = fast_ica_parallel_compute (X1);
    // w <- a %*% K
    Eigen::MatrixXd w = a * K_temp2;
    // S <- w %*% X
    S = w * X;
    // A <- t(w) %*% solve(w %*% t(w))
    A = w.transpose () * (w * w.transpose ()).inverse ();
    A.transposeInPlace ();
    K = K_temp2;
    W = a;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

Eigen::MatrixXd FastICA::fast_ica_parallel_compute (const Eigen::MatrixXd &X)
{
    int cols = (int)X.cols ();
    Eigen::MatrixXd W (num_components, num_components);
    random_normal (W);

    //  sW <- La.svd(W)
    Eigen::BDCSVD<Eigen::MatrixXd> sW (W, Eigen::ComputeThinU | Eigen::ComputeThinV);
    // W <- sW$u %*% Diag(1/sW$d) %*% t(sW$u) %*% W
    W = sW.matrixU () * (sW.singularValues ().array ().inverse ()).matrix ().asDiagonal () *
        sW.matrixU ().transpose () * W;
    Eigen::MatrixXd W1 = W;
    // lim <- rep(1000, maxit)
    std::vector<double> lim (max_it, 1000);
    // iteration counter
    int it = 0;

    while (lim[it] > tol && it < (max_it - 1))
    {
        //  wx <- W %*% X
        // gwx <- tanh(alpha * wx)
        // alpha = 1 , so ignore
        Eigen::MatrixXd gwx = (W * X).array ().tanh ().matrix ();
        // v1 <- gwx %*% t(X)/cols
        Eigen::MatrixXd v1 = gwx * (X.array () / cols).matrix ().transpose ();
        // g.wx <- alpha * (1 - (gwx)^2)
        // nb alpha == 1
        gwx = 1 - gwx.array ().square ();
        // v2 <- Diag(apply(g.wx, 1, FUN = mean)) %*% W
        Eigen::MatrixXd v2 = gwx.array ().rowwise ().mean ().matrix ().asDiagonal () * W;
        // W1 <- v1 - v2
        W1 = v1 - v2;
        // sW1 <- La.svd(W1)
        Eigen::BDCSVD<Eigen::MatrixXd> sW1 (W1, Eigen::ComputeThinU | Eigen::ComputeThinV);
        // W1 <- sW1$u %*% Diag(1/sW1$d) %*% t(sW1$u) %*% W1
        W1 = sW1.matrixU () * (sW1.singularValues ().array ().inverse ()).matrix ().asDiagonal () *
            sW1.matrixU ().transpose () * W1;
        // lim[it + 1] <- max( Mod(   Mod(  diag(W1 %*% t(W) )  )  - 1 ) )
        lim[it + 1] = ((W1 * W.transpose ()).diagonal ().array ().abs () - 1).abs ().maxCoeff ();
        // W <- W1
        W = W1;
        ++it;
    }

    return W;
}

void FastICA::scale (Eigen::Ref<Eigen::MatrixXd> M, bool center, bool normalize,
    bool ignore_invariants, std::vector<int> *zeros)
{
    int rows = (int)M.rows ();
    Eigen::Array<double, 1, Eigen::Dynamic> means = M.rowwise ().mean ();

    if (normalize)
    {
        // TODO: This block has not been tested yet
        Eigen::Array<double, 1, Eigen::Dynamic> sds =
            ((M.array ().rowwise () - means).square ().colwise ().sum () / (rows - 1)).sqrt ();

        for (int i = 0; i < sds.size (); i++)
        {
            if (sds[i] == 0)
            {
                if (!ignore_invariants)
                    return;
                if (zeros != NULL)
                    zeros->push_back (i);
                sds[i] = 1.0;
            }
        }

        if (center)
        {
            M.array ().rowwise () -= means;
        }
        M.array ().rowwise () /= sds;
    }
    else
    {
        M.array ().transpose ().rowwise () -= means;
    }
}

void FastICA::random_normal (Eigen::MatrixXd &M)
{
    std::random_device rd {};
    std::mt19937 gen {rd ()};
    std::normal_distribution<double> d {0, 1};

    for (int r = 0; r < M.rows (); r++)
    {
        for (int c = 0; c < M.cols (); c++)
        {
            M (r, c) = d (gen);
        }
    }
}

int FastICA::get_matrixes (double *w_mat, double *k_mat, double *a_mat, double *s_mat)
{
    for (int r = 0; r < (int)W.rows (); r++)
    {
        for (int c = 0; c < (int)W.cols (); c++)
        {
            w_mat[r * W.cols () + c] = W (r, c);
        }
    }
    for (int r = 0; r < (int)S.rows (); r++)
    {
        for (int c = 0; c < (int)S.cols (); c++)
        {
            s_mat[r * S.cols () + c] = S (r, c);
        }
    }
    for (int r = 0; r < (int)K.rows (); r++)
    {
        for (int c = 0; c < (int)K.cols (); c++)
        {
            k_mat[r * K.cols () + c] = K (r, c);
        }
    }
    for (int r = 0; r < (int)A.rows (); r++)
    {
        for (int c = 0; c < (int)A.cols (); c++)
        {
            a_mat[r * A.cols () + c] = A (r, c);
        }
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}
