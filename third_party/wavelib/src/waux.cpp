#include "waux.h"
#include "../header/wauxlib.h"

int compare_double (const void *a, const void *b)
{
    double arg1 = *(const double *)a;
    double arg2 = *(const double *)b;

    if (arg1 < arg2)
        return -1;
    if (arg1 > arg2)
        return 1;
    return 0;
}

double mean (const double *vec, int N)
{
    int i;
    double m;
    m = 0.0;

    for (i = 0; i < N; ++i)
    {
        m += vec[i];
    }
    m = m / N;
    return m;
}

double var (const double *vec, int N)
{
    double v, temp, m;
    int i;
    v = 0.0;
    m = mean (vec, N);

    for (i = 0; i < N; ++i)
    {
        temp = vec[i] - m;
        v += temp * temp;
    }

    v = v / N;

    return v;
}

double median (double *x, int N)
{
    double sigma;

    qsort (x, N, sizeof (double), compare_double);

    if ((N % 2) == 0)
    {
        sigma = (x[N / 2 - 1] + x[N / 2]) / 2.0;
    }
    else
    {
        sigma = x[N / 2];
    }

    return sigma;
}

double mad (double *x, int N)
{
    double sigma;
    int i;

    sigma = median (x, N);

    for (i = 0; i < N; ++i)
    {
        x[i] = (x[i] - sigma) > 0 ? (x[i] - sigma) : -(x[i] - sigma);
    }

    sigma = median (x, N);

    return sigma;
}

int minindex (const double *arr, int N)
{
    double min;
    int index, i;

    min = DBL_MAX;
    index = 0;
    for (i = 0; i < N; ++i)
    {
        if (arr[i] < min)
        {
            min = arr[i];
            index = i;
        }
    }

    return index;
}

void getDWTAppx (wt_object wt, double *appx, int N)
{
    /*
    Wavelet decomposition is stored as
    [A(J) D(J) D(J-1) ..... D(1)] in wt->output vector

    Length of A(J) , N = wt->length[0]
    */
    int i;

    for (i = 0; i < N; ++i)
    {
        appx[i] = wt->output[i];
    }
}

void getDWTDetail (wt_object wt, double *detail, int N, int level)
{
    /*
    returns Detail coefficents at the jth level where j = J,J-1,...,1
    and Wavelet decomposition is stored as
    [A(J) D(J) D(J-1) ..... D(1)] in wt->output vector
    Use getDWTAppx() to get A(J)
    Level 1 : Length of D(J), ie N, is stored in wt->length[1]
    Level 2 :Length of D(J-1), ie N, is stored in wt->length[2]
    ....
    Level J : Length of D(1), ie N, is stored in wt->length[J]
    */
    int i, iter, J;
    J = wt->J;

    if (level > J || level < 1)
    {
        throw std::runtime_error ("invalid decomposition size");
    }

    iter = wt->length[0];

    for (i = 1; i < J - level; ++i)
    {
        iter += wt->length[i];
    }

    for (i = 0; i < N; ++i)
    {
        detail[i] = wt->output[i + iter];
    }
}

void getDWTRecCoeff (double *coeff, int *length, const char *ctype, const char *ext, int level,
    int J, double *lpr, double *hpr, int lf, int siglength, double *reccoeff)
{

    int i, j, k, det_len, N, l, m, n, v, t, l2;
    double *out, *X_lp, *filt;
    out = (double *)malloc (sizeof (double) * (siglength + 1));
    l2 = lf / 2;
    m = -2;
    n = -1;
    if (!strcmp (ext, "per"))
    {
        if (!strcmp ((ctype), "appx"))
        {
            det_len = length[0];
        }
        else
        {
            det_len = length[J - level + 1];
        }

        N = 2 * length[J];

        X_lp = (double *)malloc (sizeof (double) * (N + 2 * lf - 1));

        for (i = 0; i < det_len; ++i)
        {
            out[i] = coeff[i];
        }

        for (j = level; j > 0; --j)
        {

            // idwt1(wt, temp, cA_up, out, det_len, wt->output + iter, det_len, X_lp, X_hp, out);

            if (!strcmp ((ctype), "det") && j == level)
            {
                filt = hpr;
            }
            else
            {
                filt = lpr;
            }

            // idwt_per(wt,out, det_len, wt->output + iter, det_len, X_lp);
            m = -2;
            n = -1;

            for (i = 0; i < det_len + l2 - 1; ++i)
            {
                m += 2;
                n += 2;
                X_lp[m] = 0.0;
                X_lp[n] = 0.0;
                for (l = 0; l < l2; ++l)
                {
                    t = 2 * l;
                    if ((i - l) >= 0 && (i - l) < det_len)
                    {
                        X_lp[m] += filt[t] * out[i - l];
                        X_lp[n] += filt[t + 1] * out[i - l];
                    }
                    else if ((i - l) >= det_len && (i - l) < det_len + lf - 1)
                    {
                        X_lp[m] += filt[t] * out[i - l - det_len];
                        X_lp[n] += filt[t + 1] * out[i - l - det_len];
                    }
                    else if ((i - l) < 0 && (i - l) > -l2)
                    {
                        X_lp[m] += filt[t] * out[det_len + i - l];
                        X_lp[n] += filt[t + 1] * out[det_len + i - l];
                    }
                }
            }

            for (k = lf / 2 - 1; k < 2 * det_len + lf / 2 - 1; ++k)
            {
                out[k - lf / 2 + 1] = X_lp[k];
            }

            if (j != 1)
            {
                det_len = length[J - j + 2];
            }
        }

        free (X_lp);
    }
    else if (!strcmp (ext, "sym"))
    {
        if (!strcmp ((ctype), "appx"))
        {
            det_len = length[0];
        }
        else
        {
            det_len = length[J - level + 1];
        }

        N = 2 * length[J] - 1;

        X_lp = (double *)malloc (sizeof (double) * (N + 2 * lf - 1));

        for (i = 0; i < det_len; ++i)
        {
            out[i] = coeff[i];
        }

        for (j = level; j > 0; --j)
        {

            // idwt1(wt, temp, cA_up, out, det_len, wt->output + iter, det_len, X_lp, X_hp, out);

            if (!strcmp ((ctype), "det") && j == level)
            {
                filt = hpr;
            }
            else
            {
                filt = lpr;
            }

            // idwt_sym(wt, out, det_len, wt->output + iter, det_len, X_lp);

            m = -2;
            n = -1;

            for (v = 0; v < det_len; ++v)
            {
                i = v;
                m += 2;
                n += 2;
                X_lp[m] = 0.0;
                X_lp[n] = 0.0;
                for (l = 0; l < lf / 2; ++l)
                {
                    t = 2 * l;
                    if ((i - l) >= 0 && (i - l) < det_len)
                    {
                        X_lp[m] += filt[t] * out[i - l];
                        X_lp[n] += filt[t + 1] * out[i - l];
                    }
                }
            }

            for (k = lf - 2; k < 2 * det_len; ++k)
            {
                out[k - lf + 2] = X_lp[k];
            }


            if (j != 1)
            {
                det_len = length[J - j + 2];
            }
        }

        free (X_lp);
    }
    else
    {
        free (out);
        throw std::runtime_error ("wrong signal extension");
    }

    for (i = 0; i < siglength; ++i)
    {
        reccoeff[i] = out[i];
    }

    free (out);
}


void autocovar (const double *vec, int N, double *acov, int M)
{
    double m, temp1, temp2;
    int i, t;
    m = mean (vec, N);

    if (M > N)
    {
        M = N - 1;
        printf ("\n Lag is greater than the length N of the input vector. It is automatically set "
                "to length N - 1.\n");
        printf ("\n The Output Vector only contains N calculated values.");
    }
    else if (M < 0)
    {
        M = 0;
    }

    for (i = 0; i < M; i++)
    {
        acov[i] = 0.0;
        for (t = 0; t < N - i; t++)
        {
            temp1 = vec[t] - m;
            temp2 = vec[t + i] - m;
            acov[i] += temp1 * temp2;
        }
        acov[i] = acov[i] / N;
    }
}

void autocorr (const double *vec, int N, double *acorr, int M)
{
    double var;
    int i;
    if (M > N)
    {
        M = N - 1;
        printf ("\n Lag is greater than the length N of the input vector. It is automatically set "
                "to length N - 1.\n");
        printf ("\n The Output Vector only contains N calculated values.");
    }
    else if (M < 0)
    {
        M = 0;
    }
    autocovar (vec, N, acorr, M);
    var = acorr[0];
    acorr[0] = 1.0;

    for (i = 1; i < M; i++)
    {
        acorr[i] = acorr[i] / var;
    }
}
