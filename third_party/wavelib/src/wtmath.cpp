/*
Copyright (c) 2018, Rafat Hussain
*/
#include "wtmath.h"

void dwt_per_stride (double *inp, int N, double *lpd, double *hpd, int lpd_len, double *cA,
    int len_cA, double *cD, int istride, int ostride)
{
    int l, l2, isodd, i, t, len_avg, is, os;

    len_avg = lpd_len;
    l2 = len_avg / 2;
    isodd = N % 2;

    for (i = 0; i < len_cA; ++i)
    {
        t = 2 * i + l2;
        os = i * ostride;
        cA[os] = 0.0;
        cD[os] = 0.0;
        for (l = 0; l < len_avg; ++l)
        {
            if ((t - l) >= l2 && (t - l) < N)
            {
                is = (t - l) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - l) < l2 && (t - l) >= 0)
            {
                is = (t - l) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - l) < 0 && isodd == 0)
            {
                is = (t - l + N) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - l) < 0 && isodd == 1)
            {
                if ((t - l) != -1)
                {
                    is = (t - l + N + 1) * istride;
                    cA[os] += lpd[l] * inp[is];
                    cD[os] += hpd[l] * inp[is];
                }
                else
                {
                    is = (N - 1) * istride;
                    cA[os] += lpd[l] * inp[is];
                    cD[os] += hpd[l] * inp[is];
                }
            }
            else if ((t - l) >= N && isodd == 0)
            {
                is = (t - l - N) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - l) >= N && isodd == 1)
            {
                is = (t - l - (N + 1)) * istride;
                if (t - l != N)
                {
                    cA[os] += lpd[l] * inp[is];
                    cD[os] += hpd[l] * inp[is];
                }
                else
                {
                    is = (N - 1) * istride;
                    cA[os] += lpd[l] * inp[is];
                    cD[os] += hpd[l] * inp[is];
                }
            }
        }
    }
}

void dwt_sym_stride (double *inp, int N, double *lpd, double *hpd, int lpd_len, double *cA,
    int len_cA, double *cD, int istride, int ostride)
{
    int i, l, t, len_avg;
    int is, os;
    len_avg = lpd_len;

    for (i = 0; i < len_cA; ++i)
    {
        t = 2 * i + 1;
        os = i * ostride;
        cA[os] = 0.0;
        cD[os] = 0.0;
        for (l = 0; l < len_avg; ++l)
        {
            if ((t - l) >= 0 && (t - l) < N)
            {
                is = (t - l) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - l) < 0)
            {
                is = (-t + l - 1) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - l) >= N)
            {
                is = (2 * N - t + l - 1) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
        }
    }
}

void modwt_per_stride (int M, double *inp, int N, double *filt, int lpd_len, double *cA, int len_cA,
    double *cD, int istride, int ostride)
{
    int l, i, t, len_avg;
    int is, os;
    len_avg = lpd_len;


    for (i = 0; i < len_cA; ++i)
    {
        t = i;
        os = i * ostride;
        is = t * istride;
        cA[os] = filt[0] * inp[is];
        cD[os] = filt[len_avg] * inp[is];
        for (l = 1; l < len_avg; l++)
        {
            t -= M;
            while (t >= len_cA)
            {
                t -= len_cA;
            }
            while (t < 0)
            {
                t += len_cA;
            }
            os = i * ostride;
            is = t * istride;
            cA[os] += filt[l] * inp[is];
            cD[os] += filt[len_avg + l] * inp[is];
        }
    }
}

void swt_per_stride (int M, double *inp, int N, double *lpd, double *hpd, int lpd_len, double *cA,
    int len_cA, double *cD, int istride, int ostride)
{
    int l, l2, isodd, i, t, len_avg, j;
    int is, os;
    len_avg = M * lpd_len;
    l2 = len_avg / 2;
    isodd = N % 2;

    for (i = 0; i < len_cA; ++i)
    {
        t = i + l2;
        os = i * ostride;
        cA[os] = 0.0;
        cD[os] = 0.0;
        l = -1;
        for (j = 0; j < len_avg; j += M)
        {
            l++;
            while (j >= len_cA)
            {
                j -= len_cA;
            }
            if ((t - j) >= l2 && (t - j) < N)
            {
                is = (t - j) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - j) < l2 && (t - j) >= 0)
            {
                is = (t - j) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - j) < 0)
            {
                is = (t - j + N) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - j) >= N && isodd == 0)
            {
                is = (t - j - N) * istride;
                cA[os] += lpd[l] * inp[is];
                cD[os] += hpd[l] * inp[is];
            }
            else if ((t - j) >= N && isodd == 1)
            {
                if (t - l != N)
                {
                    is = (t - j - (N + 1)) * istride;
                    cA[os] += lpd[l] * inp[is];
                    cD[os] += hpd[l] * inp[is];
                }
                else
                {
                    is = (N - 1) * istride;
                    cA[os] += lpd[l] * inp[is];
                    cD[os] += hpd[l] * inp[N - 1];
                }
            }
        }
    }
}

void idwt_per_stride (double *cA, int len_cA, double *cD, double *lpr, double *hpr, int lpr_len,
    double *X, int istride, int ostride)
{
    int len_avg, i, l, m, n, t, l2;
    int is, ms, ns;

    len_avg = lpr_len;
    l2 = len_avg / 2;
    m = -2;
    n = -1;

    for (i = 0; i < len_cA + l2 - 1; ++i)
    {
        m += 2;
        n += 2;
        ms = m * ostride;
        ns = n * ostride;
        X[ms] = 0.0;
        X[ns] = 0.0;
        for (l = 0; l < l2; ++l)
        {
            t = 2 * l;
            if ((i - l) >= 0 && (i - l) < len_cA)
            {
                is = (i - l) * istride;
                X[ms] += lpr[t] * cA[is] + hpr[t] * cD[is];
                X[ns] += lpr[t + 1] * cA[is] + hpr[t + 1] * cD[is];
            }
            else if ((i - l) >= len_cA && (i - l) < len_cA + len_avg - 1)
            {
                is = (i - l - len_cA) * istride;
                X[ms] += lpr[t] * cA[is] + hpr[t] * cD[is];
                X[ns] += lpr[t + 1] * cA[is] + hpr[t + 1] * cD[is];
            }
            else if ((i - l) < 0 && (i - l) > -l2)
            {
                is = (len_cA + i - l) * istride;
                X[ms] += lpr[t] * cA[is] + hpr[t] * cD[is];
                X[ns] += lpr[t + 1] * cA[is] + hpr[t + 1] * cD[is];
            }
        }
    }
}

void idwt_sym_stride (double *cA, int len_cA, double *cD, double *lpr, double *hpr, int lpr_len,
    double *X, int istride, int ostride)
{
    int len_avg, i, l, m, n, t, v;
    int ms, ns, is;
    len_avg = lpr_len;
    m = -2;
    n = -1;

    for (v = 0; v < len_cA; ++v)
    {
        i = v;
        m += 2;
        n += 2;
        ms = m * ostride;
        ns = n * ostride;
        X[ms] = 0.0;
        X[ns] = 0.0;
        for (l = 0; l < len_avg / 2; ++l)
        {
            t = 2 * l;
            if ((i - l) >= 0 && (i - l) < len_cA)
            {
                is = (i - l) * istride;
                X[ms] += lpr[t] * cA[is] + hpr[t] * cD[is];
                X[ns] += lpr[t + 1] * cA[is] + hpr[t + 1] * cD[is];
            }
        }
    }
}

void imodwt_per_stride (int M, double *cA, int len_cA, double *cD, double *filt, int lf, double *X,
    int istride, int ostride)
{
    int len_avg, i, l, t;
    int is, os;

    len_avg = lf;

    for (i = 0; i < len_cA; ++i)
    {
        t = i;
        os = i * ostride;
        is = t * istride;
        X[os] = (filt[0] * cA[is]) + (filt[len_avg] * cD[is]);
        for (l = 1; l < len_avg; l++)
        {
            t += M;
            while (t >= len_cA)
            {
                t -= len_cA;
            }
            while (t < 0)
            {
                t += len_cA;
            }
            is = t * istride;
            X[os] += (filt[l] * cA[is]) + (filt[len_avg + l] * cD[is]);
        }
    }
}

void idwt2_shift (int shift, int rows, int cols, double *lpr, double *hpr, int lf, double *A,
    double *H, double *V, double *D, double *oup)
{
    int i, k, N, ir, ic, J, dim1, dim2;
    int istride, ostride;
    double *cL, *cH, *X_lp;


    N = rows > cols ? 2 * rows : 2 * cols;

    J = 1;
    dim1 = 2 * rows;
    dim2 = 2 * cols;

    X_lp = (double *)malloc (sizeof (double) * (N + 2 * lf - 1));
    cL = (double *)calloc (dim1 * dim2, sizeof (double));
    cH = (double *)calloc (dim1 * dim2, sizeof (double));

    ir = rows;
    ic = cols;
    istride = ic;
    ostride = 1;
    for (i = 0; i < ic; ++i)
    {
        idwt_per_stride (A + i, ir, H + i, lpr, hpr, lf, X_lp, istride, ostride);

        for (k = lf / 2 - 1; k < 2 * ir + lf / 2 - 1; ++k)
        {
            cL[(k - lf / 2 + 1) * ic + i] = X_lp[k];
        }

        idwt_per_stride (V + i, ir, D + i, lpr, hpr, lf, X_lp, istride, ostride);

        for (k = lf / 2 - 1; k < 2 * ir + lf / 2 - 1; ++k)
        {
            cH[(k - lf / 2 + 1) * ic + i] = X_lp[k];
        }
    }
    ir *= 2;
    istride = 1;
    ostride = 1;

    for (i = 0; i < ir; ++i)
    {
        idwt_per_stride (cL + i * ic, ic, cH + i * ic, lpr, hpr, lf, X_lp, istride, ostride);

        for (k = lf / 2 - 1; k < 2 * ic + lf / 2 - 1; ++k)
        {
            oup[(k - lf / 2 + 1) + i * ic * 2] = X_lp[k];
        }
    }
    ic *= 2;


    if (shift == -1)
    {
        // Save the last column
        for (i = 0; i < ir; ++i)
        {
            cL[i] = oup[(i + 1) * ic - 1];
        }
        // Save the last row
        memcpy (cH, oup + (ir - 1) * ic, sizeof (double) * ic);
        for (i = ir - 1; i > 0; --i)
        {
            memcpy (oup + i * ic + 1, oup + (i - 1) * ic, sizeof (double) * (ic - 1));
        }
        oup[0] = cL[ir - 1];
        for (i = 1; i < ir; ++i)
        {
            oup[i * ic] = cL[i - 1];
        }

        for (i = 1; i < ic; ++i)
        {
            oup[i] = cH[i - 1];
        }
    }


    free (X_lp);
    free (cL);
    free (cH);
}

int upsamp (double *x, int lenx, int M, double *y)
{
    int N, i, j, k;

    if (M < 0)
    {
        return -1;
    }

    if (M == 0)
    {
        for (i = 0; i < lenx; ++i)
        {
            y[i] = x[i];
        }
        return lenx;
    }

    N = M * (lenx - 1) + 1;
    j = 1;
    k = 0;

    for (i = 0; i < N; ++i)
    {
        j--;
        y[i] = 0.0;
        if (j == 0)
        {
            y[i] = x[k];
            k++;
            j = M;
        }
    }

    return N;
}

int upsamp2 (double *x, int lenx, int M, double *y)
{
    int N, i, j, k;
    // upsamp2 returns even numbered output. Last value is set to zero
    if (M < 0)
    {
        return -1;
    }

    if (M == 0)
    {
        for (i = 0; i < lenx; ++i)
        {
            y[i] = x[i];
        }
        return lenx;
    }

    N = M * lenx;
    j = 1;
    k = 0;

    for (i = 0; i < N; ++i)
    {
        j--;
        y[i] = 0.0;
        if (j == 0)
        {
            y[i] = x[k];
            k++;
            j = M;
        }
    }

    return N;
}

int downsamp (double *x, int lenx, int M, double *y)
{
    int N, i;

    if (M < 0)
    {
        return -1;
    }
    if (M == 0)
    {
        for (i = 0; i < lenx; ++i)
        {
            y[i] = x[i];
        }
        return lenx;
    }

    N = (lenx - 1) / M + 1;

    for (i = 0; i < N; ++i)
    {
        y[i] = x[i * M];
    }

    return N;
}
/*
int per_ext(double *sig, int len, int a,double *oup) {
    int i,len2;
    // oup is of length len + (len%2) + 2 * a
    for (i = 0; i < len; ++i) {
        oup[a + i] = sig[i];
    }
    len2 = len;
    if ((len % 2) != 0) {
        len2 = len + 1;
        oup[a + len] = sig[len - 1];
    }
    for (i = 0; i < a; ++i) {
        oup[a-1-i] = sig[len - 1 - i];
        oup[len2 + a + i] = sig[i];
    }

    return len2;

}
*/

int per_ext (double *sig, int len, int a, double *oup)
{
    int i, len2;
    double temp1;
    double temp2;
    for (i = 0; i < len; ++i)
    {
        oup[a + i] = sig[i];
    }
    len2 = len;
    if ((len % 2) != 0)
    {
        len2 = len + 1;
        oup[a + len] = sig[len - 1];
    }
    for (i = 0; i < a; ++i)
    {
        temp1 = oup[a + i];
        temp2 = oup[a + len2 - 1 - i];
        oup[a - 1 - i] = temp2;
        oup[len2 + a + i] = temp1;
    }
    return len2;
}
/*
int symm_ext(double *sig, int len, int a, double *oup) {
    int i, len2;
    // oup is of length len + 2 * a
    for (i = 0; i < len; ++i) {
        oup[a + i] = sig[i];
    }
    len2 = len;
    for (i = 0; i < a; ++i) {
        oup[a - 1 - i] = sig[i];
        oup[len2 + a + i] = sig[len - 1 - i];
    }

    return len2;

}
*/

int symm_ext (double *sig, int len, int a, double *oup)
{
    int i, len2;
    double temp1;
    double temp2;
    // oup is of length len + 2 * a
    for (i = 0; i < len; ++i)
    {
        oup[a + i] = sig[i];
    }
    len2 = len;
    for (i = 0; i < a; ++i)
    {
        temp1 = oup[a + i];
        temp2 = oup[a + len2 - 1 - i];
        oup[a - 1 - i] = temp1;
        oup[len2 + a + i] = temp2;
    }

    return len2;
}

static int isign (int N)
{
    int M;
    if (N >= 0)
    {
        M = 1;
    }
    else
    {
        M = -1;
    }

    return M;
}

static int iabs (int N)
{
    if (N >= 0)
    {
        return N;
    }
    else
    {
        return -N;
    }
}

void circshift (double *array, int N, int L)
{
    int i;
    double *temp;
    if (iabs (L) > N)
    {
        L = isign (L) * (iabs (L) % N);
    }
    if (L < 0)
    {
        L = (N + L) % N;
    }

    temp = (double *)malloc (sizeof (double) * L);

    for (i = 0; i < L; ++i)
    {
        temp[i] = array[i];
    }

    for (i = 0; i < N - L; ++i)
    {
        array[i] = array[i + L];
    }

    for (i = 0; i < L; ++i)
    {
        array[N - L + i] = temp[i];
    }

    free (temp);
}

int testSWTlength (int N, int J)
{
    int ret, div, i;
    ret = 1;

    div = 1;
    for (i = 0; i < J; ++i)
    {
        div *= 2;
    }

    if (N % div)
    {
        ret = 0;
    }

    return ret;
}

int wmaxiter (int sig_len, int filt_len)
{
    int lev;
    double temp;

    temp = log ((double)sig_len / ((double)filt_len - 1.0)) / log (2.0);
    lev = (int)temp;

    return lev;
}

static double entropy_s (double *x, int N)
{
    int i;
    double val, x2;

    val = 0.0;

    for (i = 0; i < N; ++i)
    {
        if (x[i] != 0)
        {
            x2 = x[i] * x[i];
            val -= x2 * log (x2);
        }
    }
    return val;
}

static double entropy_t (double *x, int N, double t)
{
    int i;
    double val, x2;
    if (t < 0)
    {
        throw std::runtime_error ("wavelib error");
    }
    val = 0.0;

    for (i = 0; i < N; ++i)
    {
        x2 = fabs (x[i]);
        if (x2 > t)
        {
            val += 1;
        }
    }

    return val;
}

static double entropy_n (double *x, int N, double p)
{
    int i;
    double val, x2;
    if (p < 1)
    {
        throw std::runtime_error ("wavelib error");
    }
    val = 0.0;
    for (i = 0; i < N; ++i)
    {
        x2 = fabs (x[i]);
        val += pow (x2, (double)p);
    }

    return val;
}

static double entropy_l (double *x, int N)
{
    int i;
    double val, x2;

    val = 0.0;

    for (i = 0; i < N; ++i)
    {
        if (x[i] != 0)
        {
            x2 = x[i] * x[i];
            val += log (x2);
        }
    }
    return val;
}

double costfunc (double *x, int N, char *entropy, double p)
{
    double val;

    if (!strcmp (entropy, "shannon"))
    {
        val = entropy_s (x, N);
    }
    else if (!strcmp (entropy, "threshold"))
    {
        val = entropy_t (x, N, p);
    }
    else if (!strcmp (entropy, "norm"))
    {
        val = entropy_n (x, N, p);
    }
    else if (!strcmp (entropy, "logenergy") || !strcmp (entropy, "log energy") ||
        !strcmp (entropy, "energy"))
    {
        val = entropy_l (x, N);
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }

    return val;
}
