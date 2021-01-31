/*
  Copyright (c) 2014, Rafat Hussain
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header/wavelib.h"
#include "cwt.h"
#include "wtmath.h"

wave_object wave_init (const char *wname)
{
    wave_object obj = NULL;
    int retval;
    retval = 0;

    if (wname != NULL)
    {
        retval = filtlength (wname);
        // obj->filtlength = retval;
        // strcopy(obj->wname, wname);
    }

    obj = (wave_object)malloc (sizeof (struct wave_set) + sizeof (double) * 4 * retval);

    obj->filtlength = retval;
    obj->lpd_len = obj->hpd_len = obj->lpr_len = obj->hpr_len = obj->filtlength;
    strcpy (obj->wname, wname);
    if (wname != NULL)
    {
        filtcoef (wname, obj->params, obj->params + retval, obj->params + 2 * retval,
            obj->params + 3 * retval);
    }
    obj->lpd = &obj->params[0];
    obj->hpd = &obj->params[retval];
    obj->lpr = &obj->params[2 * retval];
    obj->hpr = &obj->params[3 * retval];
    return obj;
}

wt_object wt_init (wave_object wave, const char *method, int siglength, int J)
{
    int size, i, MaxIter;
    wt_object obj = NULL;

    size = wave->filtlength;

    if (J > 100)
    {
        throw std::runtime_error ("decomposition iterations can not be bigger than 100");
    }

    MaxIter = wmaxiter (siglength, size);

    if (J > MaxIter)
    {
        throw std::runtime_error ("number of data points is too small for this wavelet");
    }

    if (method == NULL)
    {
        obj = (wt_object)malloc (
            sizeof (struct wt_set) + sizeof (double) * (siglength + 2 * J * (size + 1)));
        obj->outlength = siglength + 2 * J * (size + 1); // Default
        strcpy (obj->ext, "sym");                        // Default
    }
    else if (!strcmp (method, "dwt") || !strcmp (method, "DWT"))
    {
        obj = (wt_object)malloc (
            sizeof (struct wt_set) + sizeof (double) * (siglength + 2 * J * (size + 1)));
        obj->outlength = siglength + 2 * J * (size + 1); // Default
        strcpy (obj->ext, "sym");                        // Default
    }
    else if (!strcmp (method, "swt") || !strcmp (method, "SWT"))
    {
        if (!testSWTlength (siglength, J))
        {
            throw std::runtime_error ("for swt signal must be a power of 2^J");
        }

        obj = (wt_object)malloc (sizeof (struct wt_set) + sizeof (double) * (siglength * (J + 1)));
        obj->outlength = siglength * (J + 1); // Default
        strcpy (obj->ext, "per");             // Default
    }
    else if (!strcmp (method, "modwt") || !strcmp (method, "MODWT"))
    {

        if (!strstr (wave->wname, "haar"))
        {
            if (!strstr (wave->wname, "db"))
            {
                if (!strstr (wave->wname, "sym"))
                {
                    if (!strstr (wave->wname, "coif"))
                    {
                        throw std::runtime_error (
                            "modwt is only implemented for orthogonal wavelet families");
                    }
                }
            }
        }

        obj = (wt_object)malloc (
            sizeof (struct wt_set) + sizeof (double) * (siglength * 2 * (J + 1)));
        obj->outlength = siglength * (J + 1); // Default
        strcpy (obj->ext, "per");             // Default
    }

    obj->wave = wave;
    obj->siglength = siglength;
    obj->modwtsiglength = siglength;
    obj->J = J;
    obj->MaxIter = MaxIter;
    strcpy (obj->method, method);

    if (siglength % 2 == 0)
    {
        obj->even = 1;
    }
    else
    {
        obj->even = 0;
    }

    obj->cobj = NULL;

    strcpy (obj->cmethod, "direct"); // Default
    obj->cfftset = 0;
    obj->lenlength = J + 2;
    obj->output = &obj->params[0];
    if (!strcmp (method, "dwt") || !strcmp (method, "DWT"))
    {
        for (i = 0; i < siglength + 2 * J * (size + 1); ++i)
        {
            obj->params[i] = 0.0;
        }
    }
    else if (!strcmp (method, "swt") || !strcmp (method, "SWT"))
    {
        for (i = 0; i < siglength * (J + 1); ++i)
        {
            obj->params[i] = 0.0;
        }
    }
    else if (!strcmp (method, "modwt") || !strcmp (method, "MODWT"))
    {
        for (i = 0; i < siglength * 2 * (J + 1); ++i)
        {
            obj->params[i] = 0.0;
        }
    }
    // wave_summary(obj->wave);

    return obj;
}

wtree_object wtree_init (wave_object wave, int siglength, int J)
{
    int size, i, MaxIter, temp, temp2, elength, nodes;
    wtree_object obj = NULL;

    size = wave->filtlength;

    if (J > 100)
    {
        throw std::runtime_error ("decomposition iterations can not exceed 100");
    }


    MaxIter = wmaxiter (siglength, size);
    if (J > MaxIter)
    {
        throw std::runtime_error ("num data points is too small for this wavelet");
    }
    temp = 1;
    elength = 0;
    nodes = 0;
    for (i = 0; i < J; ++i)
    {
        temp *= 2;
        nodes += temp;
        temp2 = (size - 2) * (temp - 1);
        elength += temp2;
    }

    obj = (wtree_object)malloc (sizeof (struct wtree_set) +
        sizeof (double) * (siglength * (J + 1) + elength + nodes + J + 1));
    obj->outlength = siglength * (J + 1) + elength;
    strcpy (obj->ext, "sym");

    obj->wave = wave;
    obj->siglength = siglength;
    obj->J = J;
    obj->MaxIter = MaxIter;
    strcpy (obj->method, "dwt");

    if (siglength % 2 == 0)
    {
        obj->even = 1;
    }
    else
    {
        obj->even = 0;
    }

    obj->cobj = NULL;
    obj->nodes = nodes;

    obj->cfftset = 0;
    obj->lenlength = J + 2;
    obj->output = &obj->params[0];
    obj->nodelength = (int *)&obj->params[siglength * (J + 1) + elength];
    obj->coeflength = (int *)&obj->params[siglength * (J + 1) + elength + nodes];

    for (i = 0; i < siglength * (J + 1) + elength + nodes + J + 1; ++i)
    {
        obj->params[i] = 0.0;
    }

    // wave_summary(obj->wave);

    return obj;
}

wpt_object wpt_init (wave_object wave, int siglength, int J)
{
    int size, i, MaxIter, temp, nodes, elength, p2, N, lp;
    wpt_object obj = NULL;

    size = wave->filtlength;

    if (J > 100)
    {
        throw std::runtime_error ("decomposition iterations can not exceed 100");
    }


    MaxIter = wmaxiter (siglength, size);
    if (J > MaxIter)
    {
        throw std::runtime_error ("num data points is too small for this wavelet");
    }
    temp = 1;
    nodes = 0;
    for (i = 0; i < J; ++i)
    {
        temp *= 2;
        nodes += temp;
    }

    i = J;
    p2 = 2;
    N = siglength;
    lp = size;
    elength = 0;
    while (i > 0)
    {
        N = N + lp - 2;
        N = (int)ceil ((double)N / 2.0);
        elength = p2 * N;
        i--;
        p2 *= 2;
    }
    // printf("elength %d", elength);

    obj = (wpt_object)malloc (
        sizeof (struct wpt_set) + sizeof (double) * (elength + 4 * nodes + 2 * J + 6));
    obj->outlength = siglength + 2 * (J + 1) * (size + 1);
    strcpy (obj->ext, "sym");
    strcpy (obj->entropy, "shannon");
    obj->eparam = 0.0;

    obj->wave = wave;
    obj->siglength = siglength;
    obj->J = J;
    obj->MaxIter = MaxIter;

    if (siglength % 2 == 0)
    {
        obj->even = 1;
    }
    else
    {
        obj->even = 0;
    }

    obj->cobj = NULL;
    obj->nodes = nodes;

    obj->lenlength = J + 2;
    obj->output = &obj->params[0];
    obj->costvalues = &obj->params[elength];
    obj->basisvector = &obj->params[elength + nodes + 1];
    obj->nodeindex = (int *)&obj->params[elength + 2 * nodes + 2];
    obj->numnodeslevel = (int *)&obj->params[elength + 4 * nodes + 4];
    obj->coeflength = (int *)&obj->params[elength + 4 * nodes + J + 5];

    for (i = 0; i < elength + 4 * nodes + 2 * J + 6; ++i)
    {
        obj->params[i] = 0.0;
    }

    // wave_summary(obj->wave);

    return obj;
}

cwt_object cwt_init (const char *wave, double param, int siglength, double dt, int J)
{
    cwt_object obj = NULL;
    int N = 0, i = 0, nj2 = 0, ibase2 = 0, mother = 0;
    double s0 = 0.0, dj = 0.0;
    double t1 = 0.0;
    int m = 0, odd = 0;
    const char *pdefault = "pow";

    m = (int)param;
    odd = 1;
    if (2 * (m / 2) == m)
    {
        odd = 0;
    }

    N = siglength;
    nj2 = 2 * N * J;
    obj = (cwt_object)malloc (sizeof (struct cwt_set) + sizeof (double) * (nj2 + 2 * J + N));

    if (!strcmp (wave, "morlet") || !strcmp (wave, "morl"))
    {
        s0 = 2 * dt;
        dj = 0.4875;
        mother = 0;
        if (param < 0.0)
        {
            free (obj);
            throw std::runtime_error ("morlet wavelet parameter should be >= 0");
        }
        if (param == 0)
        {
            param = 6.0;
        }
        strcpy (obj->wave, "morlet");
    }
    else if (!strcmp (wave, "paul"))
    {
        s0 = 2 * dt;
        dj = 0.4875;
        mother = 1;
        if (param < 0 || param > 20)
        {
            free (obj);
            throw std::runtime_error ("Paul wavelet parameter should be >0 and <= 20");
        }
        if (param == 0)
        {
            param = 4.0;
        }
        strcpy (obj->wave, "paul");
    }
    else if (!strcmp (wave, "dgauss") || !strcmp (wave, "dog"))
    {
        s0 = 2 * dt;
        dj = 0.4875;
        mother = 2;
        if (param < 0 || odd == 1)
        {
            free (obj);
            throw std::runtime_error ("DOF wavelet parameter should be > 0 and even");
        }
        if (param == 0)
        {
            param = 2.0;
        }
        strcpy (obj->wave, "dog");
    }

    obj->pow = 2;
    strcpy (obj->type, pdefault);

    obj->s0 = s0;
    obj->dj = dj;
    obj->dt = dt;
    obj->J = J;
    obj->siglength = siglength;
    obj->sflag = 0;
    obj->pflag = 1;
    obj->mother = mother;
    obj->m = param;

    t1 = 0.499999 + log ((double)N) / log (2.0);
    ibase2 = 1 + (int)t1;

    obj->npad = (int)pow (2.0, (double)ibase2);

    obj->output = (cplx_data *)&obj->params[0];
    obj->scale = &obj->params[nj2];
    obj->period = &obj->params[nj2 + J];
    obj->coi = &obj->params[nj2 + 2 * J];

    for (i = 0; i < nj2 + 2 * J + N; ++i)
    {
        obj->params[i] = 0.0;
    }

    return obj;
}

wt2_object wt2_init (wave_object wave, const char *method, int rows, int cols, int J)
{
    int size, i, MaxIter, MaxRows, MaxCols, sumacc;
    wt2_object obj = NULL;

    size = wave->filtlength;

    MaxRows = wmaxiter (rows, size);
    MaxCols = wmaxiter (cols, size);

    MaxIter = (MaxRows < MaxCols) ? MaxRows : MaxCols;

    if (J > MaxIter)
    {
        throw std::runtime_error ("wavelib error");
    }

    if (J == 1)
    {
        sumacc = 4;
    }
    else if (J > 1)
    {
        sumacc = J * 3 + 1;
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }

    if (method == NULL)
    {
        obj = (wt2_object)malloc (sizeof (struct wt2_set) + sizeof (int) * (2 * J + sumacc));
        obj->outlength = 0; // Default
        strcpy (obj->ext, "per");
    }
    else if (!strcmp (method, "dwt") || !strcmp (method, "DWT"))
    {
        obj = (wt2_object)malloc (sizeof (struct wt2_set) + sizeof (int) * (2 * J + sumacc));
        obj->outlength = 0; // Default
        strcpy (obj->ext, "per");
    }
    else if (!strcmp (method, "swt") || !strcmp (method, "SWT"))
    {
        if (!testSWTlength (rows, J) || !testSWTlength (cols, J))
        {
            throw std::runtime_error ("wavelib error");
        }

        obj = (wt2_object)malloc (sizeof (struct wt2_set) + sizeof (int) * (2 * J + sumacc));
        obj->outlength = 0; // Default
        strcpy (obj->ext, "per");
    }
    else if (!strcmp (method, "modwt") || !strcmp (method, "MODWT"))
    {
        if (!strstr (wave->wname, "haar"))
        {
            if (!strstr (wave->wname, "db"))
            {
                if (!strstr (wave->wname, "sym"))
                {
                    if (!strstr (wave->wname, "coif"))
                    {
                        throw std::runtime_error ("wavelib error");
                    }
                }
            }
        }
        obj = (wt2_object)malloc (sizeof (struct wt2_set) + sizeof (int) * (2 * J + sumacc));
        obj->outlength = 0; // Default
        strcpy (obj->ext, "per");
    }


    obj->wave = wave;
    obj->rows = rows;
    obj->cols = cols;
    obj->J = J;
    obj->MaxIter = MaxIter;
    strcpy (obj->method, method);
    obj->coeffaccesslength = sumacc;

    obj->dimensions = &obj->params[0];
    obj->coeffaccess = &obj->params[2 * J];
    for (i = 0; i < (2 * J + sumacc); ++i)
    {
        obj->params[i] = 0;
    }

    return obj;
}


static void wconv (wt_object wt, double *sig, int N, double *filt, int L, double *oup)
{
    if (!strcmp (wt->cmethod, "direct"))
    {
        conv_direct (sig, N, filt, L, oup);
    }
    else if (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT"))
    {
        if (wt->cfftset == 0)
        {
            wt->cobj = conv_init (N, L);
            conv_fft (wt->cobj, sig, filt, oup);
            free_conv (wt->cobj);
        }
        else
        {
            conv_fft (wt->cobj, sig, filt, oup);
        }
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}


static void dwt_per (wt_object wt, double *inp, int N, double *cA, int len_cA, double *cD)
{

    dwt_per_stride (inp, N, wt->wave->lpd, wt->wave->hpd, wt->wave->lpd_len, cA, len_cA, cD, 1, 1);
}

static void wtree_per (wtree_object wt, double *inp, int N, double *cA, int len_cA, double *cD)
{
    int l, l2, isodd, i, t, len_avg;

    len_avg = wt->wave->lpd_len;
    l2 = len_avg / 2;
    isodd = N % 2;

    for (i = 0; i < len_cA; ++i)
    {
        t = 2 * i + l2;
        cA[i] = 0.0;
        cD[i] = 0.0;
        for (l = 0; l < len_avg; ++l)
        {
            if ((t - l) >= l2 && (t - l) < N)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l];
                cD[i] += wt->wave->hpd[l] * inp[t - l];
            }
            else if ((t - l) < l2 && (t - l) >= 0)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l];
                cD[i] += wt->wave->hpd[l] * inp[t - l];
            }
            else if ((t - l) < 0 && isodd == 0)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l + N];
                cD[i] += wt->wave->hpd[l] * inp[t - l + N];
            }
            else if ((t - l) < 0 && isodd == 1)
            {
                if ((t - l) != -1)
                {
                    cA[i] += wt->wave->lpd[l] * inp[t - l + N + 1];
                    cD[i] += wt->wave->hpd[l] * inp[t - l + N + 1];
                }
                else
                {
                    cA[i] += wt->wave->lpd[l] * inp[N - 1];
                    cD[i] += wt->wave->hpd[l] * inp[N - 1];
                }
            }
            else if ((t - l) >= N && isodd == 0)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l - N];
                cD[i] += wt->wave->hpd[l] * inp[t - l - N];
            }
            else if ((t - l) >= N && isodd == 1)
            {
                if (t - l != N)
                {
                    cA[i] += wt->wave->lpd[l] * inp[t - l - (N + 1)];
                    cD[i] += wt->wave->hpd[l] * inp[t - l - (N + 1)];
                }
                else
                {
                    cA[i] += wt->wave->lpd[l] * inp[N - 1];
                    cD[i] += wt->wave->hpd[l] * inp[N - 1];
                }
            }
        }
    }
}

static void dwpt_per (wpt_object wt, double *inp, int N, double *cA, int len_cA, double *cD)
{
    int l, l2, isodd, i, t, len_avg;

    len_avg = wt->wave->lpd_len;
    l2 = len_avg / 2;
    isodd = N % 2;

    for (i = 0; i < len_cA; ++i)
    {
        t = 2 * i + l2;
        cA[i] = 0.0;
        cD[i] = 0.0;
        for (l = 0; l < len_avg; ++l)
        {
            if ((t - l) >= l2 && (t - l) < N)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l];
                cD[i] += wt->wave->hpd[l] * inp[t - l];
            }
            else if ((t - l) < l2 && (t - l) >= 0)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l];
                cD[i] += wt->wave->hpd[l] * inp[t - l];
            }
            else if ((t - l) < 0 && isodd == 0)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l + N];
                cD[i] += wt->wave->hpd[l] * inp[t - l + N];
            }
            else if ((t - l) < 0 && isodd == 1)
            {
                if ((t - l) != -1)
                {
                    cA[i] += wt->wave->lpd[l] * inp[t - l + N + 1];
                    cD[i] += wt->wave->hpd[l] * inp[t - l + N + 1];
                }
                else
                {
                    cA[i] += wt->wave->lpd[l] * inp[N - 1];
                    cD[i] += wt->wave->hpd[l] * inp[N - 1];
                }
            }
            else if ((t - l) >= N && isodd == 0)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l - N];
                cD[i] += wt->wave->hpd[l] * inp[t - l - N];
            }
            else if ((t - l) >= N && isodd == 1)
            {
                if (t - l != N)
                {
                    cA[i] += wt->wave->lpd[l] * inp[t - l - (N + 1)];
                    cD[i] += wt->wave->hpd[l] * inp[t - l - (N + 1)];
                }
                else
                {
                    cA[i] += wt->wave->lpd[l] * inp[N - 1];
                    cD[i] += wt->wave->hpd[l] * inp[N - 1];
                }
            }
        }
    }
}

static void dwt_sym (wt_object wt, double *inp, int N, double *cA, int len_cA, double *cD)
{

    dwt_sym_stride (inp, N, wt->wave->lpd, wt->wave->hpd, wt->wave->lpd_len, cA, len_cA, cD, 1, 1);
}

static void wtree_sym (wtree_object wt, double *inp, int N, double *cA, int len_cA, double *cD)
{
    int i, l, t, len_avg;

    len_avg = wt->wave->lpd_len;

    for (i = 0; i < len_cA; ++i)
    {
        t = 2 * i + 1;
        cA[i] = 0.0;
        cD[i] = 0.0;
        for (l = 0; l < len_avg; ++l)
        {
            if ((t - l) >= 0 && (t - l) < N)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l];
                cD[i] += wt->wave->hpd[l] * inp[t - l];
            }
            else if ((t - l) < 0)
            {
                cA[i] += wt->wave->lpd[l] * inp[-t + l - 1];
                cD[i] += wt->wave->hpd[l] * inp[-t + l - 1];
            }
            else if ((t - l) >= N)
            {
                cA[i] += wt->wave->lpd[l] * inp[2 * N - t + l - 1];
                cD[i] += wt->wave->hpd[l] * inp[2 * N - t + l - 1];
            }
        }
    }
}

static void dwpt_sym (wpt_object wt, double *inp, int N, double *cA, int len_cA, double *cD)
{
    int i, l, t, len_avg;

    len_avg = wt->wave->lpd_len;

    for (i = 0; i < len_cA; ++i)
    {
        t = 2 * i + 1;
        cA[i] = 0.0;
        cD[i] = 0.0;
        for (l = 0; l < len_avg; ++l)
        {
            if ((t - l) >= 0 && (t - l) < N)
            {
                cA[i] += wt->wave->lpd[l] * inp[t - l];
                cD[i] += wt->wave->hpd[l] * inp[t - l];
            }
            else if ((t - l) < 0)
            {
                cA[i] += wt->wave->lpd[l] * inp[-t + l - 1];
                cD[i] += wt->wave->hpd[l] * inp[-t + l - 1];
            }
            else if ((t - l) >= N)
            {
                cA[i] += wt->wave->lpd[l] * inp[2 * N - t + l - 1];
                cD[i] += wt->wave->hpd[l] * inp[2 * N - t + l - 1];
            }
        }
    }
}

static void dwt1 (wt_object wt, double *sig, int len_sig, double *cA, double *cD)
{
    int len_avg, D, lf;
    double *signal, *cA_undec;
    len_avg = (wt->wave->lpd_len + wt->wave->hpd_len) / 2;
    // len_sig = 2 * (int)ceil((double)len_sig / 2.0);

    D = 2;

    if (!strcmp (wt->ext, "per"))
    {
        signal = (double *)malloc (sizeof (double) * (len_sig + len_avg + (len_sig % 2)));

        len_sig = per_ext (sig, len_sig, len_avg / 2, signal);

        cA_undec = (double *)malloc (sizeof (double) * (len_sig + len_avg + wt->wave->lpd_len - 1));

        if (wt->wave->lpd_len == wt->wave->hpd_len &&
            (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
        {
            wt->cobj = conv_init (len_sig + len_avg, wt->wave->lpd_len);
            wt->cfftset = 1;
        }
        else if (!(wt->wave->lpd_len == wt->wave->hpd_len))
        {
            free (cA_undec);
            throw std::runtime_error ("decomposition filters must have the same lenth");
        }

        wconv (wt, signal, len_sig + len_avg, wt->wave->lpd, wt->wave->lpd_len, cA_undec);

        downsamp (cA_undec + len_avg, len_sig, D, cA);

        wconv (wt, signal, len_sig + len_avg, wt->wave->hpd, wt->wave->hpd_len, cA_undec);

        downsamp (cA_undec + len_avg, len_sig, D, cD);
    }
    else if (!strcmp (wt->ext, "sym"))
    {
        // printf("\n YES %s \n", wt->ext);
        lf = wt->wave->lpd_len; // lpd and hpd have the same length

        signal = (double *)malloc (sizeof (double) * (len_sig + 2 * (lf - 1)));

        len_sig = symm_ext (sig, len_sig, lf - 1, signal);

        cA_undec = (double *)malloc (sizeof (double) * (len_sig + 3 * (lf - 1)));

        if (wt->wave->lpd_len == wt->wave->hpd_len &&
            (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
        {
            wt->cobj = conv_init (len_sig + 2 * (lf - 1), lf);
            wt->cfftset = 1;
        }
        else if (!(wt->wave->lpd_len == wt->wave->hpd_len))
        {
            free (cA_undec);
            throw std::runtime_error ("decomposition filters must have the same length");
        }


        wconv (wt, signal, len_sig + 2 * (lf - 1), wt->wave->lpd, wt->wave->lpd_len, cA_undec);

        downsamp (cA_undec + lf, len_sig + lf - 2, D, cA);

        wconv (wt, signal, len_sig + 2 * (lf - 1), wt->wave->hpd, wt->wave->hpd_len, cA_undec);

        downsamp (cA_undec + lf, len_sig + lf - 2, D, cD);
    }
    else
    {
        throw std::runtime_error ("signal externsion must be sym or per");
    }


    if (wt->wave->lpd_len == wt->wave->hpd_len &&
        (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
    {
        free_conv (wt->cobj);
        wt->cfftset = 0;
    }

    free (signal);
    free (cA_undec);
}

void dwt (wt_object wt, const double *inp)
{
    int i, J, temp_len, iter, N, lp;
    int len_cA;
    double *orig, *orig2;

    temp_len = wt->siglength;
    J = wt->J;
    wt->length[J + 1] = temp_len;
    wt->outlength = 0;
    wt->zpad = 0;
    orig = (double *)malloc (sizeof (double) * temp_len);
    orig2 = (double *)malloc (sizeof (double) * temp_len);
    /*
    if ((temp_len % 2) == 0) {
    wt->zpad = 0;
    orig = (double*)malloc(sizeof(double)* temp_len);
    orig2 = (double*)malloc(sizeof(double)* temp_len);
    }
    else {
    wt->zpad = 1;
    temp_len++;
    orig = (double*)malloc(sizeof(double)* temp_len);
    orig2 = (double*)malloc(sizeof(double)* temp_len);
    }
    */

    for (i = 0; i < wt->siglength; ++i)
    {
        orig[i] = inp[i];
    }

    if (wt->zpad == 1)
    {
        orig[temp_len - 1] = orig[temp_len - 2];
    }

    N = temp_len;
    lp = wt->wave->lpd_len;

    if (!strcmp (wt->ext, "per"))
    {
        i = J;
        while (i > 0)
        {
            N = (int)ceil ((double)N / 2.0);
            wt->length[i] = N;
            wt->outlength += wt->length[i];
            i--;
        }
        wt->length[0] = wt->length[1];
        wt->outlength += wt->length[0];
        N = wt->outlength;

        for (iter = 0; iter < J; ++iter)
        {
            len_cA = wt->length[J - iter];
            N -= len_cA;
            if (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT"))
            {
                dwt1 (wt, orig, temp_len, orig2, wt->params + N);
            }
            else
            {
                dwt_per (wt, orig, temp_len, orig2, len_cA, wt->params + N);
            }
            temp_len = wt->length[J - iter];
            if (iter == J - 1)
            {
                for (i = 0; i < len_cA; ++i)
                {
                    wt->params[i] = orig2[i];
                }
            }
            else
            {
                for (i = 0; i < len_cA; ++i)
                {
                    orig[i] = orig2[i];
                }
            }
        }
    }
    else if (!strcmp (wt->ext, "sym"))
    {
        // printf("\n YES %s \n", wt->ext);
        i = J;
        while (i > 0)
        {
            N = N + lp - 2;
            N = (int)ceil ((double)N / 2.0);
            wt->length[i] = N;
            wt->outlength += wt->length[i];
            i--;
        }
        wt->length[0] = wt->length[1];
        wt->outlength += wt->length[0];
        N = wt->outlength;

        for (iter = 0; iter < J; ++iter)
        {
            len_cA = wt->length[J - iter];
            N -= len_cA;
            if (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT"))
            {
                dwt1 (wt, orig, temp_len, orig2, wt->params + N);
            }
            else
            {
                dwt_sym (wt, orig, temp_len, orig2, len_cA, wt->params + N);
            }
            temp_len = wt->length[J - iter];

            if (iter == J - 1)
            {
                for (i = 0; i < len_cA; ++i)
                {
                    wt->params[i] = orig2[i];
                }
            }
            else
            {
                for (i = 0; i < len_cA; ++i)
                {
                    orig[i] = orig2[i];
                }
            }
        }
    }
    else
    {
        throw std::runtime_error ("signal extension must be sym or per");
    }

    free (orig);
    free (orig2);
}

static void getDWTRecCoeff (double *coeff, int *length, const char *ctype, const char *ext,
    int level, int J, double *lpr, double *hpr, int lf, int siglength, double *reccoeff)
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

            if (!strcmp ((ctype), "det") && j == level)
            {
                filt = hpr;
            }
            else
            {
                filt = lpr;
            }

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

            if (!strcmp ((ctype), "det") && j == level)
            {
                filt = hpr;
            }
            else
            {
                filt = lpr;
            }

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
        throw std::runtime_error ("wavelib error");
    }

    for (i = 0; i < siglength; ++i)
    {
        reccoeff[i] = out[i];
    }

    free (out);
}


double *getDWTmra (wt_object wt, double *wavecoeffs)
{
    int i, J, access, N;
    double *mra;
    J = wt->J;
    mra = (double *)malloc (sizeof (double) * wt->siglength * (J + 1));
    access = 0;


    // Approximation MRA
    getDWTRecCoeff (wt->output + access, wt->length, "appx", wt->ext, J, J, wt->wave->lpr,
        wt->wave->hpr, wt->wave->lpr_len, wt->siglength, mra);

    // Details MRA
    N = wt->siglength;
    for (i = J; i > 0; --i)
    {
        access += wt->length[J - i];
        getDWTRecCoeff (wt->output + access, wt->length, "det", wt->ext, i, J, wt->wave->lpr,
            wt->wave->hpr, wt->wave->lpr_len, wt->siglength, mra + N);
        N += wt->siglength;
    }

    return mra;
}

void wtree (wtree_object wt, const double *inp)
{
    int i, J, temp_len, iter, N, lp, p2, k, N2, Np;
    int len_cA, t, t2, it1;
    double *orig;

    temp_len = wt->siglength;
    J = wt->J;
    wt->length[J + 1] = temp_len;
    wt->outlength = 0;
    wt->zpad = 0;
    orig = (double *)malloc (sizeof (double) * temp_len);
    /*
    if ((temp_len % 2) == 0) {
        wt->zpad = 0;
        orig = (double*)malloc(sizeof(double)* temp_len);
    }
    else {
        wt->zpad = 1;
        temp_len++;
        orig = (double*)malloc(sizeof(double)* temp_len);
    }
    */
    for (i = 0; i < wt->siglength; ++i)
    {
        orig[i] = inp[i];
    }

    if (wt->zpad == 1)
    {
        orig[temp_len - 1] = orig[temp_len - 2];
    }

    N = temp_len;
    lp = wt->wave->lpd_len;
    p2 = 1;

    if (!strcmp (wt->ext, "per"))
    {
        i = J;
        p2 = 2;
        while (i > 0)
        {
            N = (int)ceil ((double)N / 2.0);
            wt->length[i] = N;
            wt->outlength += p2 * (wt->length[i]);
            i--;
            p2 *= 2;
        }
        wt->length[0] = wt->length[1];

        N2 = N = wt->outlength;
        p2 = 1;
        for (iter = 0; iter < J; ++iter)
        {
            len_cA = wt->length[J - iter];
            N2 -= 2 * p2 * len_cA;
            N = N2;
            for (k = 0; k < p2; ++k)
            {
                if (iter == 0)
                {
                    wtree_per (wt, orig, temp_len, wt->params + N, len_cA, wt->params + N + len_cA);
                }
                else
                {
                    wtree_per (wt, wt->params + Np + k * temp_len, temp_len, wt->params + N, len_cA,
                        wt->params + N + len_cA);
                }
                N += 2 * len_cA;
            }

            temp_len = wt->length[J - iter];
            p2 = 2 * p2;
            Np = N2;
        }
    }
    else if (!strcmp (wt->ext, "sym"))
    {
        // printf("\n YES %s \n", wt->ext);
        i = J;
        p2 = 2;
        while (i > 0)
        {
            N = N + lp - 2;
            N = (int)ceil ((double)N / 2.0);
            wt->length[i] = N;
            wt->outlength += p2 * (wt->length[i]);
            i--;
            p2 *= 2;
        }
        wt->length[0] = wt->length[1];

        N2 = N = wt->outlength;
        p2 = 1;

        for (iter = 0; iter < J; ++iter)
        {
            len_cA = wt->length[J - iter];
            N2 -= 2 * p2 * len_cA;
            N = N2;
            for (k = 0; k < p2; ++k)
            {
                if (iter == 0)
                {
                    wtree_sym (wt, orig, temp_len, wt->params + N, len_cA, wt->params + N + len_cA);
                }
                else
                {
                    wtree_sym (wt, wt->params + Np + k * temp_len, temp_len, wt->params + N, len_cA,
                        wt->params + N + len_cA);
                }
                N += 2 * len_cA;
            }

            temp_len = wt->length[J - iter];
            p2 = 2 * p2;
            Np = N2;
        }
    }
    else
    {
        free (orig);
        throw std::runtime_error ("wavelib error");
    }

    J = wt->J;
    t2 = wt->outlength - 2 * wt->length[J];
    p2 = 2;
    it1 = 0;
    for (i = 0; i < J; ++i)
    {
        t = t2;
        for (k = 0; k < p2; ++k)
        {
            wt->nodelength[it1] = t;
            it1++;
            t += wt->length[J - i];
        }
        p2 *= 2;
        t2 = t2 - p2 * wt->length[J - i - 1];
    }

    wt->coeflength[0] = wt->siglength;

    for (i = 1; i < J + 1; ++i)
    {
        wt->coeflength[i] = wt->length[J - i + 1];
    }

    free (orig);
}

static int ipow2 (int n)
{
    int p, i;
    p = 1;

    for (i = 0; i < n; ++i)
    {
        p *= 2;
    }

    return p;
}

void dwpt (wpt_object wt, const double *inp)
{
    int i, J, temp_len, iter, N, lp, p2, k, N2, Np;
    int temp, elength, temp2, size, nodes, llb, n1, j;
    double eparam, v1, v2;
    int len_cA, t, t2, it1, it2;
    double *orig, *tree;
    int *nodelength;

    temp_len = wt->siglength;
    J = wt->J;
    wt->length[J + 1] = temp_len;
    wt->outlength = 0;
    temp = 1;
    elength = 0;
    size = wt->wave->filtlength;
    nodes = wt->nodes;
    n1 = nodes + 1;
    for (i = 0; i < J; ++i)
    {
        temp *= 2;
        temp2 = (size - 2) * (temp - 1);
        elength += temp2;
    }
    eparam = wt->eparam;
    orig = (double *)malloc (sizeof (double) * temp_len);
    tree = (double *)malloc (sizeof (double) * (temp_len * (J + 1) + elength));
    nodelength = (int *)malloc (sizeof (int) * nodes);

    for (i = 0; i < wt->siglength; ++i)
    {
        orig[i] = inp[i];
    }

    for (i = 0; i < temp_len * (J + 1) + elength; ++i)
    {
        tree[i] = 0.0;
    }

    for (i = 0; i < nodes + 1; ++i)
    {
        wt->basisvector[i] = 0.0;
        wt->costvalues[i] = 0.0;
    }

    N = temp_len;
    lp = wt->wave->lpd_len;
    p2 = 1;

    // set eparam value here
    wt->costvalues[0] = costfunc (orig, wt->siglength, wt->entropy, eparam);
    it2 = 1;
    if (!strcmp (wt->ext, "per"))
    {
        i = J;
        p2 = 2;
        while (i > 0)
        {
            N = (int)ceil ((double)N / 2.0);
            wt->length[i] = N;
            wt->outlength += p2 * (wt->length[i]);
            i--;
            p2 *= 2;
        }
        wt->length[0] = wt->length[1];

        N2 = N = wt->outlength;
        p2 = 1;
        for (iter = 0; iter < J; ++iter)
        {
            len_cA = wt->length[J - iter];
            N2 -= 2 * p2 * len_cA;
            N = N2;
            for (k = 0; k < p2; ++k)
            {
                if (iter == 0)
                {
                    dwpt_per (wt, orig, temp_len, tree + N, len_cA, tree + N + len_cA);
                }
                else
                {
                    dwpt_per (wt, tree + Np + k * temp_len, temp_len, tree + N, len_cA,
                        tree + N + len_cA);
                }
                wt->costvalues[it2] = costfunc (tree + N, len_cA, wt->entropy, eparam);
                it2++;
                wt->costvalues[it2] = costfunc (tree + N + len_cA, len_cA, wt->entropy, eparam);
                it2++;
                N += 2 * len_cA;
            }

            temp_len = wt->length[J - iter];
            p2 = 2 * p2;
            Np = N2;
        }
    }
    else if (!strcmp (wt->ext, "sym"))
    {
        // printf("\n YES %s \n", wt->ext);
        i = J;
        p2 = 2;
        while (i > 0)
        {
            N = N + lp - 2;
            N = (int)ceil ((double)N / 2.0);
            wt->length[i] = N;
            wt->outlength += p2 * (wt->length[i]);
            i--;
            p2 *= 2;
        }
        wt->length[0] = wt->length[1];

        N2 = N = wt->outlength;
        p2 = 1;

        for (iter = 0; iter < J; ++iter)
        {
            len_cA = wt->length[J - iter];
            N2 -= 2 * p2 * len_cA;
            N = N2;
            for (k = 0; k < p2; ++k)
            {
                if (iter == 0)
                {
                    dwpt_sym (wt, orig, temp_len, tree + N, len_cA, tree + N + len_cA);
                }
                else
                {
                    dwpt_sym (wt, tree + Np + k * temp_len, temp_len, tree + N, len_cA,
                        tree + N + len_cA);
                }
                wt->costvalues[it2] = costfunc (tree + N, len_cA, wt->entropy, eparam);
                it2++;
                wt->costvalues[it2] = costfunc (tree + N + len_cA, len_cA, wt->entropy, eparam);
                it2++;
                N += 2 * len_cA;
            }

            temp_len = wt->length[J - iter];
            p2 = 2 * p2;
            Np = N2;
        }
    }
    else
    {
        free (tree);
        free (nodelength);
        throw std::runtime_error ("wavelib error");
    }

    J = wt->J;
    t2 = wt->outlength - 2 * wt->length[J];
    p2 = 2;
    it1 = 0;
    for (i = 0; i < J; ++i)
    {
        t = t2;
        for (k = 0; k < p2; ++k)
        {
            nodelength[it1] = t;
            it1++;
            t += wt->length[J - i];
        }
        p2 *= 2;
        t2 = t2 - p2 * wt->length[J - i - 1];
    }


    J = wt->J;
    llb = 1;
    for (i = 0; i < J; ++i)
    {
        llb *= 2;
    }

    for (i = n1 - llb; i < n1; ++i)
    {
        wt->basisvector[i] = 1;
    }

    for (j = J - 1; j >= 0; --j)
    {
        for (k = ipow2 (j) - 1; k < ipow2 (j + 1) - 1; ++k)
        {
            v1 = wt->costvalues[k];
            v2 = wt->costvalues[2 * k + 1] + wt->costvalues[2 * k + 2];
            // printf(" %g %g", v1,v2);
            if (v1 <= v2)
            {
                wt->basisvector[k] = 1;
            }
            else
            {
                wt->costvalues[k] = v2;
            }
        }
        // printf("\n");
    }

    for (k = 0; k < nodes / 2; ++k)
    {
        if (wt->basisvector[k] == 1 || wt->basisvector[k] == 2)
        {
            wt->basisvector[2 * k + 1] = 2;
            wt->basisvector[2 * k + 2] = 2;
        }
    }

    for (k = 0; k < n1; ++k)
    {
        if (wt->basisvector[k] == 2)
        {
            wt->basisvector[k] = 0;
        }
    }

    N2 = 0;
    it1 = n1;
    it2 = 0;
    wt->nodes = 0;
    wt->numnodeslevel[0] = 0;
    // printf("Start \n");

    if (wt->basisvector[0] == 1)
    {
        wt->outlength = wt->siglength;
        for (i = 0; i < wt->siglength; ++i)
        {
            wt->output[i] = inp[i];
        }
        wt->nodes = 1;
        wt->nodeindex[0] = 0;
        wt->nodeindex[1] = 0;
        wt->numnodeslevel[0] = 1;
    }
    else
    {
        for (i = J; i > 0; --i)
        {
            llb = ipow2 (i);
            it1 -= llb;
            wt->numnodeslevel[i] = 0;
            for (j = 0; j < llb; ++j)
            {
                if (wt->basisvector[it1 + j] == 1)
                {
                    // printf("NODE %d %d %d \n", i, j, wt->length[J - i + 1]);
                    wt->nodeindex[2 * wt->nodes] = i;
                    wt->nodeindex[2 * wt->nodes + 1] = j;
                    wt->nodes += 1;
                    wt->numnodeslevel[i] += 1;
                    for (k = 0; k < wt->length[J - i + 1]; ++k)
                    {
                        wt->output[it2 + k] = tree[nodelength[it1 - 1 + j] + k]; // access tree
                    }
                    it2 += wt->length[J - i + 1];
                }
            }
        }
        wt->outlength = it2;
    }

    wt->coeflength[0] = wt->siglength;

    for (i = 1; i < J + 1; ++i)
    {
        wt->coeflength[i] = wt->length[J - i + 1];
    }

    free (orig);
    free (tree);
    free (nodelength);
}

int getWTREENodelength (wtree_object wt, int X)
{
    int N;
    N = -1;
    /*
    X - Level. All Nodes at any level have the same length
    */
    if (X <= 0 || X > wt->J)
    {
        throw std::runtime_error ("wavelib error");
    }

    N = wt->length[wt->J - X + 1];

    return N;
}

int getDWPTNodelength (wpt_object wt, int X)
{
    int N;
    N = -1;
    /*
    X - Level. All Nodes at any level have the same length
    */
    if (X <= 0 || X > wt->J)
    {
        throw std::runtime_error ("wavelib error");
    }

    N = wt->length[wt->J - X + 1];

    return N;
}

void getWTREECoeffs (wtree_object wt, int X, int Y, double *coeffs, int N)
{
    int ymax, i, t, t2;

    if (X <= 0 || X > wt->J)
    {
        throw std::runtime_error ("wavelib error");
    }
    ymax = 1;
    for (i = 0; i < X; ++i)
    {
        ymax *= 2;
    }

    ymax -= 1;

    if (Y < 0 || Y > ymax)
    {
        throw std::runtime_error ("wavelib error");
    }

    if (X == 1)
    {
        t = 0;
    }
    else
    {
        t = 0;
        t2 = 1;
        for (i = 0; i < X - 1; ++i)
        {
            t2 *= 2;
            t += t2;
        }
    }

    t += Y;
    t2 = wt->nodelength[t];
    for (i = 0; i < N; ++i)
    {
        coeffs[i] = wt->output[t2 + i];
    }
}

void getDWPTCoeffs (wpt_object wt, int X, int Y, double *coeffs, int N)
{
    int ymax, i;
    int np, citer;
    int flag;

    if (X <= 0 || X > wt->J)
    {
        throw std::runtime_error ("wavelib error");
    }
    ymax = 1;
    for (i = 0; i < X; ++i)
    {
        ymax *= 2;
    }

    ymax -= 1;

    if (Y < 0 || Y > ymax)
    {
        throw std::runtime_error ("wavelib error");
    }

    np = 0;
    citer = 0;

    for (i = wt->J; i > X; --i)
    {
        np += wt->numnodeslevel[i];
        citer += wt->numnodeslevel[i] * wt->coeflength[i];
    }

    i = 0;
    flag = 0;
    for (i = 0; i < wt->numnodeslevel[X]; ++i)
    {
        if (wt->nodeindex[2 * np + 1] == Y)
        {
            flag = 1;
            break;
        }
        np++;
        citer += wt->coeflength[X];
    }

    if (flag == 0)
    {
        throw std::runtime_error ("wavelib error");
    }

    for (i = 0; i < N; ++i)
    {
        coeffs[i] = wt->output[citer + i];
    }
}

int getCWTScaleLength (int N)
{
    int J;
    double temp, dj;

    dj = 0.4875;

    temp = (log ((double)N / 2.0) / log (2.0)) / dj;
    J = (int)temp;

    return J;
}

void setCWTScales (cwt_object wt, double s0, double dj, const char *type, int power)
{
    int i;
    strcpy (wt->type, type);
    // s0*pow(2.0, (double)(j - 1)*dj);
    if (!strcmp (wt->type, "pow") || !strcmp (wt->type, "power"))
    {
        for (i = 0; i < wt->J; ++i)
        {
            wt->scale[i] = s0 * pow ((double)power, (double)(i)*dj);
        }
        wt->sflag = 1;
        wt->pow = power;
    }
    else if (!strcmp (wt->type, "lin") || !strcmp (wt->type, "linear"))
    {
        for (i = 0; i < wt->J; ++i)
        {
            wt->scale[i] = s0 + (double)i * dj;
        }
        wt->sflag = 1;
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
    wt->s0 = s0;
    wt->dj = dj;
}

void setCWTScaleVector (cwt_object wt, const double *scale, int J, double s0, double dj)
{
    int i;

    if (J != wt->J)
    {
        throw std::runtime_error ("wavelib error");
    }

    for (i = 0; i < wt->J; ++i)
    {
        wt->scale[i] = scale[i];
    }
    wt->dj = dj;
    wt->s0 = s0;
    wt->sflag = 1;
}

void setCWTPadding (cwt_object wt, int pad)
{
    if (pad == 0)
    {
        wt->pflag = 0;
    }
    else
    {
        wt->pflag = 1;
    }
}

void cwt (cwt_object wt, const double *inp)
{
    int i, N, npad, nj2, j, j2;
    N = wt->siglength;
    if (wt->sflag == 0)
    {
        for (i = 0; i < wt->J; ++i)
        {
            wt->scale[i] = wt->s0 * pow (2.0, (double)(i)*wt->dj);
        }
        wt->sflag = 1;
    }

    if (wt->pflag == 0)
    {
        npad = N;
    }
    else
    {
        npad = wt->npad;
    }

    nj2 = 2 * N * wt->J;
    j = wt->J;
    j2 = 2 * j;

    wt->smean = 0.0;

    for (i = 0; i < N; ++i)
    {
        wt->smean += inp[i];
    }
    wt->smean /= N;

    cwavelet (inp, N, wt->dt, wt->mother, wt->m, wt->s0, wt->dj, wt->J, npad, wt->params,
        wt->params + nj2, wt->params + nj2 + j, wt->params + nj2 + j2);
}

void icwt (cwt_object wt, double *cwtop)
{
    double psi, cdel;
    int real, i, N, nj2;

    N = wt->siglength;
    nj2 = N * 2 * wt->J;

    psi0 (wt->mother, wt->m, &psi, &real);
    cdel = cdelta (wt->mother, wt->m, psi);

    // printf("\n PSI %g CDEL %g param %g mother %d \n", psi, cdel,wt->m,wt->mother);
    if ((!strcmp (wt->type, "pow") || !strcmp (wt->type, "power")) && wt->pow == 2)
    {
        icwavelet (wt->params, N, wt->params + nj2, wt->J, wt->dt, wt->dj, cdel, psi, cwtop);
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
    for (i = 0; i < N; ++i)
    {
        cwtop[i] += wt->smean;
    }
}

static void idwt1 (wt_object wt, double *temp, double *cA_up, double *cA, int len_cA, double *cD,
    int len_cD, double *X_lp, double *X_hp, double *X)
{
    int len_avg, N, U, N2, i;

    len_avg = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;
    N = 2 * len_cD;
    U = 2;

    upsamp2 (cA, len_cA, U, cA_up);

    per_ext (cA_up, 2 * len_cA, len_avg / 2, temp);

    N2 = 2 * len_cA + len_avg;

    if (wt->wave->lpr_len == wt->wave->hpr_len &&
        (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
    {
        wt->cobj = conv_init (N2, len_avg);
        wt->cfftset = 1;
    }
    else if (!(wt->wave->lpr_len == wt->wave->hpr_len))
    {
        throw std::runtime_error ("wavelib error");
    }

    wconv (wt, temp, N2, wt->wave->lpr, len_avg, X_lp);

    upsamp2 (cD, len_cD, U, cA_up);

    per_ext (cA_up, 2 * len_cD, len_avg / 2, temp);

    N2 = 2 * len_cD + len_avg;

    wconv (wt, temp, N2, wt->wave->hpr, len_avg, X_hp);


    for (i = len_avg - 1; i < N + len_avg - 1; ++i)
    {
        X[i - len_avg + 1] = X_lp[i] + X_hp[i];
    }

    if (wt->wave->lpr_len == wt->wave->hpr_len &&
        (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
    {
        free_conv (wt->cobj);
        wt->cfftset = 0;
    }
}

static void idwt_per (wt_object wt, double *cA, int len_cA, double *cD, double *X)
{
    idwt_per_stride (cA, len_cA, cD, wt->wave->lpr, wt->wave->hpr, wt->wave->lpr_len, X, 1, 1);
}

static void idwt_sym (wt_object wt, double *cA, int len_cA, double *cD, double *X)
{
    idwt_sym_stride (cA, len_cA, cD, wt->wave->lpr, wt->wave->hpr, wt->wave->lpr_len, X, 1, 1);
}


void idwt (wt_object wt, double *dwtop)
{
    int J, U, i, lf, N, N2, iter, k;
    int app_len, det_len;
    double *cA_up, *X_lp, *X_hp, *out, *temp;

    J = wt->J;
    U = 2;
    app_len = wt->length[0];
    out = (double *)malloc (sizeof (double) * (wt->siglength + 1));
    if (!strcmp (wt->ext, "per") && (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
    {
        app_len = wt->length[0];
        det_len = wt->length[1];
        N = 2 * wt->length[J];
        lf = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;

        cA_up = (double *)malloc (sizeof (double) * N);
        temp = (double *)malloc (sizeof (double) * (N + lf));
        X_lp = (double *)malloc (sizeof (double) * (N + 2 * lf - 1));
        X_hp = (double *)malloc (sizeof (double) * (N + 2 * lf - 1));
        iter = app_len;

        for (i = 0; i < app_len; ++i)
        {
            out[i] = wt->output[i];
        }

        for (i = 0; i < J; ++i)
        {

            idwt1 (wt, temp, cA_up, out, det_len, wt->output + iter, det_len, X_lp, X_hp, out);
            /*
            idwt_per(wt,out, det_len, wt->output + iter, det_len, X_lp);
            for (k = lf/2 - 1; k < 2 * det_len + lf/2 - 1; ++k) {
                out[k - lf/2 + 1] = X_lp[k];
            }
            */
            iter += det_len;
            det_len = wt->length[i + 2];
        }
        free (cA_up);
        free (X_lp);
        free (X_hp);
        free (temp);
    }
    else if (!strcmp (wt->ext, "per") && !strcmp (wt->cmethod, "direct"))
    {
        app_len = wt->length[0];
        det_len = wt->length[1];
        N = 2 * wt->length[J];
        lf = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;

        X_lp = (double *)malloc (sizeof (double) * (N + 2 * lf - 1));
        iter = app_len;

        for (i = 0; i < app_len; ++i)
        {
            out[i] = wt->output[i];
        }

        for (i = 0; i < J; ++i)
        {

            // idwt1(wt, temp, cA_up, out, det_len, wt->output + iter, det_len, X_lp, X_hp, out);

            idwt_per (wt, out, det_len, wt->output + iter, X_lp);
            for (k = lf / 2 - 1; k < 2 * det_len + lf / 2 - 1; ++k)
            {
                out[k - lf / 2 + 1] = X_lp[k];
            }

            iter += det_len;
            det_len = wt->length[i + 2];
        }

        free (X_lp);
    }
    else if (!strcmp (wt->ext, "sym") && !strcmp (wt->cmethod, "direct"))
    {
        app_len = wt->length[0];
        det_len = wt->length[1];
        N = 2 * wt->length[J] - 1;
        lf = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;

        X_lp = (double *)malloc (sizeof (double) * (N + 2 * lf - 1));
        iter = app_len;

        for (i = 0; i < app_len; ++i)
        {
            out[i] = wt->output[i];
        }

        for (i = 0; i < J; ++i)
        {

            // idwt1(wt, temp, cA_up, out, det_len, wt->output + iter, det_len, X_lp, X_hp, out);

            idwt_sym (wt, out, det_len, wt->output + iter, X_lp);
            for (k = lf - 2; k < 2 * det_len; ++k)
            {
                out[k - lf + 2] = X_lp[k];
            }

            iter += det_len;
            det_len = wt->length[i + 2];
        }

        free (X_lp);
    }
    else if (!strcmp (wt->ext, "sym") &&
        (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
    {
        lf = wt->wave->lpd_len; // lpd and hpd have the same length

        N = 2 * wt->length[J] - 1;
        cA_up = (double *)malloc (sizeof (double) * N);
        X_lp = (double *)malloc (sizeof (double) * (N + lf - 1));
        X_hp = (double *)malloc (sizeof (double) * (N + lf - 1));

        for (i = 0; i < app_len; ++i)
        {
            out[i] = wt->output[i];
        }

        iter = app_len;

        for (i = 0; i < J; ++i)
        {
            det_len = wt->length[i + 1];
            upsamp (out, det_len, U, cA_up);
            N2 = 2 * wt->length[i + 1] - 1;

            if (wt->wave->lpr_len == wt->wave->hpr_len &&
                (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
            {
                wt->cobj = conv_init (N2, lf);
                wt->cfftset = 1;
            }
            else if (!(wt->wave->lpr_len == wt->wave->hpr_len))
            {
                throw std::runtime_error ("wavelib error");
            }

            wconv (wt, cA_up, N2, wt->wave->lpr, lf, X_lp);

            upsamp (wt->output + iter, det_len, U, cA_up);

            wconv (wt, cA_up, N2, wt->wave->hpr, lf, X_hp);

            for (k = lf - 2; k < N2 + 1; ++k)
            {
                out[k - lf + 2] = X_lp[k] + X_hp[k];
            }
            iter += det_len;
            if (wt->wave->lpr_len == wt->wave->hpr_len &&
                (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
            {
                free_conv (wt->cobj);
                wt->cfftset = 0;
            }
        }

        free (cA_up);
        free (X_lp);
        free (X_hp);
    }
    else
    {
        free (out);
        throw std::runtime_error ("wavelib error");
    }

    for (i = 0; i < wt->siglength; ++i)
    {
        dwtop[i] = out[i];
    }


    free (out);
}

static void idwpt_per (wpt_object wt, double *cA, int len_cA, double *cD, double *X)
{
    int len_avg, i, l, m, n, t, l2;

    len_avg = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;
    l2 = len_avg / 2;
    m = -2;
    n = -1;

    for (i = 0; i < len_cA + l2 - 1; ++i)
    {
        m += 2;
        n += 2;
        X[m] = 0.0;
        X[n] = 0.0;
        for (l = 0; l < l2; ++l)
        {
            t = 2 * l;
            if ((i - l) >= 0 && (i - l) < len_cA)
            {
                X[m] += wt->wave->lpr[t] * cA[i - l] + wt->wave->hpr[t] * cD[i - l];
                X[n] += wt->wave->lpr[t + 1] * cA[i - l] + wt->wave->hpr[t + 1] * cD[i - l];
            }
            else if ((i - l) >= len_cA && (i - l) < len_cA + len_avg - 1)
            {
                X[m] +=
                    wt->wave->lpr[t] * cA[i - l - len_cA] + wt->wave->hpr[t] * cD[i - l - len_cA];
                X[n] += wt->wave->lpr[t + 1] * cA[i - l - len_cA] +
                    wt->wave->hpr[t + 1] * cD[i - l - len_cA];
            }
            else if ((i - l) < 0 && (i - l) > -l2)
            {
                X[m] +=
                    wt->wave->lpr[t] * cA[len_cA + i - l] + wt->wave->hpr[t] * cD[len_cA + i - l];
                X[n] += wt->wave->lpr[t + 1] * cA[len_cA + i - l] +
                    wt->wave->hpr[t + 1] * cD[len_cA + i - l];
            }
        }
    }
}

static void idwpt_sym (wpt_object wt, double *cA, int len_cA, double *cD, double *X)
{
    int len_avg, i, l, m, n, t, v;

    len_avg = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;
    m = -2;
    n = -1;

    for (v = 0; v < len_cA; ++v)
    {
        i = v;
        m += 2;
        n += 2;
        X[m] = 0.0;
        X[n] = 0.0;
        for (l = 0; l < len_avg / 2; ++l)
        {
            t = 2 * l;
            if ((i - l) >= 0 && (i - l) < len_cA)
            {
                X[m] += wt->wave->lpr[t] * cA[i - l] + wt->wave->hpr[t] * cD[i - l];
                X[n] += wt->wave->lpr[t + 1] * cA[i - l] + wt->wave->hpr[t + 1] * cD[i - l];
            }
        }
    }
}

void idwpt (wpt_object wt, double *dwtop)
{
    int J, i, lf, k, p, l;
    int app_len, det_len, index, n1, llb, index2, index3, index4, indexp, xlen;
    double *X_lp, *X, *out, *out2;
    int *prep, *ptemp;

    J = wt->J;
    app_len = wt->length[0];
    p = ipow2 (J);
    lf = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;
    xlen = p * (app_len + 2 * lf);

    X_lp = (double *)malloc (sizeof (double) * 2 * (wt->length[J] + lf));
    X = (double *)malloc (sizeof (double) * xlen);
    out = (double *)malloc (sizeof (double) * wt->length[J]);
    out2 = (double *)malloc (sizeof (double) * wt->length[J]);
    prep = (int *)malloc (sizeof (int) * p);
    ptemp = (int *)malloc (sizeof (int) * p);
    n1 = 1;
    llb = 1;
    index2 = xlen / p;
    indexp = 0;
    if (wt->basisvector[0] == 1)
    {
        for (i = 0; i < wt->siglength; ++i)
        {
            dwtop[i] = wt->output[i];
        }
    }
    else
    {
        for (i = 0; i < J; ++i)
        {
            llb *= 2;
            n1 += llb;
        }

        for (i = 0; i < xlen; ++i)
        {
            X[i] = 0.0;
        }

        for (i = 0; i < llb; ++i)
        {
            prep[i] = (int)wt->basisvector[n1 - llb + i];
            ptemp[i] = 0;
        }

        if (!strcmp (wt->ext, "per"))
        {
            app_len = wt->length[0];
            det_len = wt->length[1];
            index = 0;


            for (i = 0; i < J; ++i)
            {
                p = ipow2 (J - i - 1);
                det_len = wt->length[i + 1];
                index2 *= 2;
                index3 = 0;
                index4 = 0;
                // idwt1(wt, temp, cA_up, out, det_len, wt->output + iter, det_len, X_lp, X_hp,
                // out);
                n1 -= llb;
                for (l = 0; l < llb; ++l)
                {
                    if (ptemp[l] != 2)
                    {
                        prep[l] = (int)wt->basisvector[n1 + l];
                    }
                    else
                    {
                        prep[l] = ptemp[l];
                    }
                    ptemp[l] = 0;
                }


                for (l = 0; l < p; ++l)
                {
                    if (prep[2 * l] == 1 && prep[2 * l + 1] == 1)
                    {
                        for (k = 0; k < det_len; ++k)
                        {
                            out[k] = wt->output[index + k];
                            out2[k] = wt->output[index + det_len + k];
                        }
                        idwpt_per (wt, out, det_len, out2, X_lp);
                        for (k = lf / 2 - 1; k < 2 * det_len + lf / 2 - 1; ++k)
                        {
                            X[index3 + k - lf / 2 + 1] = X_lp[k];
                        }
                        index += 2 * det_len;
                        index3 += index2;
                        index4 += 2 * indexp;
                        ptemp[l] = 2;
                    }
                    else if (prep[2 * l] == 1 && prep[2 * l + 1] == 2)
                    {
                        index4 += indexp;
                        for (k = 0; k < det_len; ++k)
                        {
                            out[k] = wt->output[index + k];
                            out2[k] = X[index4 + k];
                        }
                        idwpt_per (wt, out, det_len, out2, X_lp);
                        for (k = lf / 2 - 1; k < 2 * det_len + lf / 2 - 1; ++k)
                        {
                            X[index3 + k - lf / 2 + 1] = X_lp[k];
                        }
                        index += det_len;
                        index3 += index2;
                        index4 += indexp;
                        ptemp[l] = 2;
                    }
                    else if (prep[2 * l] == 2 && prep[2 * l + 1] == 1)
                    {
                        for (k = 0; k < det_len; ++k)
                        {
                            out[k] = X[index4 + k];
                            out2[k] = wt->output[index + k];
                        }
                        idwpt_per (wt, out, det_len, out2, X_lp);
                        for (k = lf / 2 - 1; k < 2 * det_len + lf / 2 - 1; ++k)
                        {
                            X[index3 + k - lf / 2 + 1] = X_lp[k];
                        }
                        index += det_len;
                        index3 += index2;
                        index4 += 2 * indexp;
                        ptemp[l] = 2;
                    }
                    else if (prep[2 * l] == 2 && prep[2 * l + 1] == 2)
                    {
                        for (k = 0; k < det_len; ++k)
                        {
                            out[k] = X[index4 + k];
                            out2[k] = X[index4 + indexp + k];
                        }
                        idwpt_per (wt, out, det_len, out2, X_lp);
                        for (k = lf / 2 - 1; k < 2 * det_len + lf / 2 - 1; ++k)
                        {
                            X[index3 + k - lf / 2 + 1] = X_lp[k];
                        }
                        index4 += 2 * indexp;
                        index3 += index2;
                        ptemp[l] = 2;
                    }
                    else
                    {
                        index3 += index2;
                        index4 += 2 * indexp;
                    }
                }


                /*
                idwt_per(wt, out, det_len, wt->output + iter, det_len, X_lp);
                for (k = lf / 2 - 1; k < 2 * det_len + lf / 2 - 1; ++k) {
                out[k - lf / 2 + 1] = X_lp[k];
                }

                iter += det_len;
                det_len = wt->length[i + 2];
                */
                llb /= 2;
                indexp = index2;
            }

            // free(X_lp);
        }
        else if (!strcmp (wt->ext, "sym"))
        {
            app_len = wt->length[0];
            det_len = wt->length[1];

            // X_lp = (double*)malloc(sizeof(double)* (N + 2 * lf - 1));
            index = 0;

            for (i = 0; i < J; ++i)
            {
                p = ipow2 (J - i - 1);
                det_len = wt->length[i + 1];
                index2 *= 2;
                index3 = 0;
                index4 = 0;
                // idwt1(wt, temp, cA_up, out, det_len, wt->output + iter, det_len, X_lp, X_hp,
                // out);
                n1 -= llb;
                for (l = 0; l < llb; ++l)
                {
                    if (ptemp[l] != 2)
                    {
                        prep[l] = (int)wt->basisvector[n1 + l];
                    }
                    else
                    {
                        prep[l] = ptemp[l];
                    }
                    ptemp[l] = 0;
                }


                for (l = 0; l < p; ++l)
                {
                    if (prep[2 * l] == 1 && prep[2 * l + 1] == 1)
                    {
                        for (k = 0; k < det_len; ++k)
                        {
                            out[k] = wt->output[index + k];
                            out2[k] = wt->output[index + det_len + k];
                        }
                        idwpt_sym (wt, out, det_len, out2, X_lp);
                        for (k = lf - 2; k < 2 * det_len; ++k)
                        {
                            X[index3 + k - lf + 2] = X_lp[k];
                        }
                        index += 2 * det_len;
                        index3 += index2;
                        index4 += 2 * indexp;
                        ptemp[l] = 2;
                    }
                    else if (prep[2 * l] == 1 && prep[2 * l + 1] == 2)
                    {
                        index4 += indexp;
                        for (k = 0; k < det_len; ++k)
                        {
                            out[k] = wt->output[index + k];
                            out2[k] = X[index4 + k];
                        }
                        idwpt_sym (wt, out, det_len, out2, X_lp);
                        for (k = lf - 2; k < 2 * det_len; ++k)
                        {
                            X[index3 + k - lf + 2] = X_lp[k];
                        }
                        index += det_len;
                        index3 += index2;
                        index4 += indexp;
                        ptemp[l] = 2;
                    }
                    else if (prep[2 * l] == 2 && prep[2 * l + 1] == 1)
                    {
                        for (k = 0; k < det_len; ++k)
                        {
                            out[k] = X[index4 + k];
                            out2[k] = wt->output[index + k];
                        }
                        idwpt_sym (wt, out, det_len, out2, X_lp);
                        for (k = lf - 2; k < 2 * det_len; ++k)
                        {
                            X[index3 + k - lf + 2] = X_lp[k];
                        }
                        index += det_len;
                        index3 += index2;
                        index4 += 2 * indexp;
                        ptemp[l] = 2;
                    }
                    else if (prep[2 * l] == 2 && prep[2 * l + 1] == 2)
                    {
                        for (k = 0; k < det_len; ++k)
                        {
                            out[k] = X[index4 + k];
                            out2[k] = X[index4 + indexp + k];
                        }
                        idwpt_sym (wt, out, det_len, out2, X_lp);
                        for (k = lf - 2; k < 2 * det_len; ++k)
                        {
                            X[index3 + k - lf + 2] = X_lp[k];
                        }
                        index4 += 2 * indexp;
                        index3 += index2;
                        ptemp[l] = 2;
                    }
                    else
                    {
                        index3 += index2;
                        index4 += 2 * indexp;
                    }
                }

                // idwt1(wt, temp, cA_up, out, det_len, wt->output + iter, det_len, X_lp, X_hp,
                // out);
                /*
                idwpt_sym(wt, out, det_len, wt->output + iter, det_len, X_lp);
                for (k = lf - 2; k < 2 * det_len; ++k) {
                out[k - lf + 2] = X_lp[k];
                }

                iter += det_len;
                det_len = wt->length[i + 2];
                */
                llb /= 2;
                indexp = index2;
            }

            // free(X_lp);
        }
        else
        {
            free (X_lp);
            free (X);
            free (out);
            free (out2);
            free (prep);
            free (ptemp);
            throw std::runtime_error ("wavelib error");
        }

        for (i = 0; i < wt->siglength; ++i)
        {
            // printf("%g ", X[i]);
            dwtop[i] = X[i];
        }
    }


    free (out);
    free (X_lp);
    free (X);
    free (out2);
    free (prep);
    free (ptemp);
}


static void swt_per (wt_object wt, int M, double *inp, int N, double *cA, int len_cA, double *cD)
{

    swt_per_stride (
        M, inp, N, wt->wave->lpd, wt->wave->hpd, wt->wave->lpd_len, cA, len_cA, cD, 1, 1);
}

static void swt_fft (wt_object wt, const double *inp)
{
    int i, J, temp_len, iter, M, N, len_filt;
    int lenacc;
    double *low_pass, *high_pass, *sig, *cA, *cD;

    temp_len = wt->siglength;
    J = wt->J;
    wt->length[0] = wt->length[J] = temp_len;
    wt->outlength = wt->length[J + 1] = (J + 1) * temp_len;
    M = 1;
    for (iter = 1; iter < J; ++iter)
    {
        M = 2 * M;
        wt->length[iter] = temp_len;
    }

    len_filt = wt->wave->filtlength;

    low_pass = (double *)malloc (sizeof (double) * M * len_filt);
    high_pass = (double *)malloc (sizeof (double) * M * len_filt);
    sig = (double *)malloc (sizeof (double) * (M * len_filt + temp_len + (temp_len % 2)));
    cA = (double *)malloc (sizeof (double) * (2 * M * len_filt + temp_len + (temp_len % 2)) - 1);
    cD = (double *)malloc (sizeof (double) * (2 * M * len_filt + temp_len + (temp_len % 2)) - 1);

    M = 1;

    for (i = 0; i < temp_len; ++i)
    {
        wt->params[i] = inp[i];
    }

    lenacc = wt->outlength;

    for (iter = 0; iter < J; ++iter)
    {
        lenacc -= temp_len;
        if (iter > 0)
        {
            M = 2 * M;
            N = M * len_filt;
            upsamp2 (wt->wave->lpd, wt->wave->lpd_len, M, low_pass);
            upsamp2 (wt->wave->hpd, wt->wave->hpd_len, M, high_pass);
        }
        else
        {
            N = len_filt;
            for (i = 0; i < N; ++i)
            {
                low_pass[i] = wt->wave->lpd[i];
                high_pass[i] = wt->wave->hpd[i];
            }
        }

        // swt_per(wt,M, wt->params, temp_len, cA, temp_len, cD,temp_len);

        per_ext (wt->params, temp_len, N / 2, sig);

        if (wt->wave->lpd_len == wt->wave->hpd_len &&
            (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
        {
            wt->cobj = conv_init (N + temp_len + (temp_len % 2), N);
            wt->cfftset = 1;
        }
        else if (!(wt->wave->lpd_len == wt->wave->hpd_len))
        {
            throw std::runtime_error ("wavelib error");
        }


        wconv (wt, sig, N + temp_len + (temp_len % 2), low_pass, N, cA);

        wconv (wt, sig, N + temp_len + (temp_len % 2), high_pass, N, cD);

        if (wt->wave->lpd_len == wt->wave->hpd_len &&
            (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
        {
            free_conv (wt->cobj);
            wt->cfftset = 0;
        }

        for (i = 0; i < temp_len; ++i)
        {
            wt->params[i] = cA[N + i];
            wt->params[lenacc + i] = cD[N + i];
        }
    }

    free (low_pass);
    free (high_pass);
    free (sig);
    free (cA);
    free (cD);
}

static void swt_direct (wt_object wt, const double *inp)
{
    int i, J, temp_len, iter, M;
    int lenacc;
    double *cA, *cD;

    temp_len = wt->siglength;
    J = wt->J;
    wt->length[0] = wt->length[J] = temp_len;
    wt->outlength = wt->length[J + 1] = (J + 1) * temp_len;
    M = 1;
    for (iter = 1; iter < J; ++iter)
    {
        M = 2 * M;
        wt->length[iter] = temp_len;
    }


    cA = (double *)malloc (sizeof (double) * temp_len);
    cD = (double *)malloc (sizeof (double) * temp_len);

    M = 1;

    for (i = 0; i < temp_len; ++i)
    {
        wt->params[i] = inp[i];
    }

    lenacc = wt->outlength;

    for (iter = 0; iter < J; ++iter)
    {
        lenacc -= temp_len;
        if (iter > 0)
        {
            M = 2 * M;
        }

        swt_per (wt, M, wt->params, temp_len, cA, temp_len, cD);


        for (i = 0; i < temp_len; ++i)
        {
            wt->params[i] = cA[i];
            wt->params[lenacc + i] = cD[i];
        }
    }

    free (cA);
    free (cD);
}


void swt (wt_object wt, const double *inp)
{
    if (!strcmp (wt->method, "swt") && !strcmp (wt->cmethod, "direct"))
    {
        swt_direct (wt, inp);
    }
    else if (!strcmp (wt->method, "swt") &&
        (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
    {
        swt_fft (wt, inp);
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}

static void getSWTRecCoeff (double *coeff, int *length, const char *ctype, int level, int J,
    double *lpr, double *hpr, int lf, int siglength, double *swtop)
{
    int N, iter, i, index, value, count, len;
    int index_shift, len0, U, N1, index2;
    double *appx1, *det1, *appx_sig, *det_sig, *cL0, *cH0, *tempx, *oup00L, *oup00H, *oup00, *oup01,
        *appx2, *det2;

    N = siglength;
    U = 2;

    appx_sig = (double *)malloc (sizeof (double) * N);
    det_sig = (double *)malloc (sizeof (double) * N);
    appx1 = (double *)malloc (sizeof (double) * N);
    det1 = (double *)malloc (sizeof (double) * N);
    appx2 = (double *)malloc (sizeof (double) * N);
    det2 = (double *)malloc (sizeof (double) * N);
    tempx = (double *)malloc (sizeof (double) * N);
    cL0 = (double *)malloc (sizeof (double) * (N + (N % 2) + lf));
    cH0 = (double *)malloc (sizeof (double) * (N + (N % 2) + lf));
    oup00L = (double *)malloc (sizeof (double) * (N + 2 * lf));
    oup00H = (double *)malloc (sizeof (double) * (N + 2 * lf));
    oup00 = (double *)malloc (sizeof (double) * N);
    oup01 = (double *)malloc (sizeof (double) * N);


    for (iter = J - level; iter < J; ++iter)
    {
        for (i = 0; i < N; ++i)
        {
            swtop[i] = 0.0;
        }
        if (!strcmp ((ctype), "appx") && (iter == (J - level)))
        {
            for (i = 0; i < N; ++i)
            {
                appx_sig[i] = coeff[i];
                det_sig[i] = 0.0;
            }
        }
        else if (!strcmp ((ctype), "det") && (iter == (J - level)))
        {
            for (i = 0; i < N; ++i)
            {
                det_sig[i] = coeff[i];
                appx_sig[i] = 0.0;
            }
        }
        else
        {
            for (i = 0; i < N; ++i)
            {
                det_sig[i] = 0.0;
            }
        }

        value = (int)pow (2.0, (double)(J - 1 - iter));

        for (count = 0; count < value; count++)
        {
            len = 0;
            for (index = count; index < N; index += value)
            {
                appx1[len] = appx_sig[index];
                det1[len] = det_sig[index];
                len++;
            }


            // SHIFT 0
            len0 = 0;

            for (index_shift = 0; index_shift < len; index_shift += 2)
            {
                appx2[len0] = appx1[index_shift];
                det2[len0] = det1[index_shift];
                len0++;
            }
            upsamp2 (appx2, len0, U, tempx);
            per_ext (tempx, 2 * len0, lf / 2, cL0);

            upsamp2 (det2, len0, U, tempx);
            per_ext (tempx, 2 * len0, lf / 2, cH0);

            N1 = 2 * len0 + lf;


            conv_direct (cL0, N1, lpr, lf, oup00L);


            conv_direct (cH0, N1, hpr, lf, oup00H);

            for (i = lf - 1; i < 2 * len0 + lf - 1; ++i)
            {
                oup00[i - lf + 1] = oup00L[i] + oup00H[i];
            }

            // SHIFT 1

            len0 = 0;

            for (index_shift = 1; index_shift < len; index_shift += 2)
            {
                appx2[len0] = appx1[index_shift];
                det2[len0] = det1[index_shift];
                len0++;
            }

            upsamp2 (appx2, len0, U, tempx);
            per_ext (tempx, 2 * len0, lf / 2, cL0);

            upsamp2 (det2, len0, U, tempx);
            per_ext (tempx, 2 * len0, lf / 2, cH0);

            N1 = 2 * len0 + lf;

            conv_direct (cL0, N1, lpr, lf, oup00L);

            conv_direct (cH0, N1, hpr, lf, oup00H);

            for (i = lf - 1; i < 2 * len0 + lf - 1; ++i)
            {
                oup01[i - lf + 1] = oup00L[i] + oup00H[i];
            }

            circshift (oup01, 2 * len0, -1);

            index2 = 0;

            for (index = count; index < N; index += value)
            {
                swtop[index] = (oup00[index2] + oup01[index2]) / 2.0;
                index2++;
            }
        }
        for (i = 0; i < N; ++i)
        {
            appx_sig[i] = swtop[i];
        }
    }


    free (appx_sig);
    free (det_sig);
    free (appx1);
    free (det1);
    free (tempx);
    free (cL0);
    free (cH0);
    free (oup00L);
    free (oup00H);
    free (oup00);
    free (oup01);
    free (appx2);
    free (det2);
}

double *getSWTmra (wt_object wt, double *wavecoeffs)
{
    int i, J, access, N;
    double *mra;
    J = wt->J;
    mra = (double *)malloc (sizeof (double) * wt->siglength * (J + 1));
    access = 0;


    // Approximation MRA
    getSWTRecCoeff (wt->output + access, wt->length, "appx", J, J, wt->wave->lpr, wt->wave->hpr,
        wt->wave->lpr_len, wt->siglength, mra);
    // Details MRA
    N = wt->siglength;

    for (i = J; i > 0; --i)
    {
        access += wt->length[J - i];
        getSWTRecCoeff (wt->output + access, wt->length, "det", i, J, wt->wave->lpr, wt->wave->hpr,
            wt->wave->lpr_len, wt->siglength, mra + N);
        N += wt->siglength;
    }

    return mra;
}

void iswt (wt_object wt, double *swtop)
{
    int N, lf, iter, i, J, index, value, count, len;
    int index_shift, len0, U, N1, index2;
    double *appx1, *det1, *appx_sig, *det_sig, *cL0, *cH0, *tempx, *oup00L, *oup00H, *oup00, *oup01,
        *appx2, *det2;

    N = wt->siglength;
    J = wt->J;
    U = 2;
    lf = wt->wave->lpr_len;

    appx_sig = (double *)malloc (sizeof (double) * N);
    det_sig = (double *)malloc (sizeof (double) * N);
    appx1 = (double *)malloc (sizeof (double) * N);
    det1 = (double *)malloc (sizeof (double) * N);
    appx2 = (double *)malloc (sizeof (double) * N);
    det2 = (double *)malloc (sizeof (double) * N);
    tempx = (double *)malloc (sizeof (double) * N);
    cL0 = (double *)malloc (sizeof (double) * (N + (N % 2) + lf));
    cH0 = (double *)malloc (sizeof (double) * (N + (N % 2) + lf));
    oup00L = (double *)malloc (sizeof (double) * (N + 2 * lf));
    oup00H = (double *)malloc (sizeof (double) * (N + 2 * lf));
    oup00 = (double *)malloc (sizeof (double) * N);
    oup01 = (double *)malloc (sizeof (double) * N);


    for (iter = 0; iter < J; ++iter)
    {
        for (i = 0; i < N; ++i)
        {
            swtop[i] = 0.0;
        }
        if (iter == 0)
        {
            for (i = 0; i < N; ++i)
            {
                appx_sig[i] = wt->output[i];
                det_sig[i] = wt->output[N + i];
            }
        }
        else
        {
            for (i = 0; i < N; ++i)
            {
                det_sig[i] = wt->output[(iter + 1) * N + i];
            }
        }

        value = (int)pow (2.0, (double)(J - 1 - iter));

        for (count = 0; count < value; count++)
        {
            len = 0;
            for (index = count; index < N; index += value)
            {
                appx1[len] = appx_sig[index];
                det1[len] = det_sig[index];
                len++;
            }


            // SHIFT 0
            len0 = 0;

            for (index_shift = 0; index_shift < len; index_shift += 2)
            {
                appx2[len0] = appx1[index_shift];
                det2[len0] = det1[index_shift];
                len0++;
            }
            upsamp2 (appx2, len0, U, tempx);
            per_ext (tempx, 2 * len0, lf / 2, cL0);

            upsamp2 (det2, len0, U, tempx);
            per_ext (tempx, 2 * len0, lf / 2, cH0);

            N1 = 2 * len0 + lf;

            if (wt->wave->lpr_len == wt->wave->hpr_len &&
                (!strcmp (wt->cmethod, "fft") || !strcmp (wt->cmethod, "FFT")))
            {
                wt->cobj = conv_init (N1, lf);
                wt->cfftset = 1;
            }
            else if (!(wt->wave->lpd_len == wt->wave->hpd_len))
            {
                free (appx1);
                free (det1);
                free (appx_sig);
                free (det_sig);
                free (oup00);
                throw std::runtime_error ("wavelib error");
            }

            wconv (wt, cL0, N1, wt->wave->lpr, lf, oup00L);

            wconv (wt, cH0, N1, wt->wave->hpr, lf, oup00H);

            for (i = lf - 1; i < 2 * len0 + lf - 1; ++i)
            {
                oup00[i - lf + 1] = oup00L[i] + oup00H[i];
            }

            // SHIFT 1

            len0 = 0;

            for (index_shift = 1; index_shift < len; index_shift += 2)
            {
                appx2[len0] = appx1[index_shift];
                det2[len0] = det1[index_shift];
                len0++;
            }

            upsamp2 (appx2, len0, U, tempx);
            per_ext (tempx, 2 * len0, lf / 2, cL0);

            upsamp2 (det2, len0, U, tempx);
            per_ext (tempx, 2 * len0, lf / 2, cH0);

            N1 = 2 * len0 + lf;

            wconv (wt, cL0, N1, wt->wave->lpr, lf, oup00L);

            wconv (wt, cH0, N1, wt->wave->hpr, lf, oup00H);

            for (i = lf - 1; i < 2 * len0 + lf - 1; ++i)
            {
                oup01[i - lf + 1] = oup00L[i] + oup00H[i];
            }

            circshift (oup01, 2 * len0, -1);

            index2 = 0;

            for (index = count; index < N; index += value)
            {
                swtop[index] = (oup00[index2] + oup01[index2]) / 2.0;
                index2++;
            }
        }
        for (i = 0; i < N; ++i)
        {
            appx_sig[i] = swtop[i];
        }
    }


    free (appx_sig);
    free (det_sig);
    free (appx1);
    free (det1);
    free (tempx);
    free (cL0);
    free (cH0);
    free (oup00L);
    free (oup00H);
    free (oup00);
    free (oup01);
    free (appx2);
    free (det2);
}

static void modwt_per (wt_object wt, int M, double *inp, double *cA, int len_cA, double *cD)
{
    int l, i, t, len_avg;
    double s;
    double *filt;
    len_avg = wt->wave->lpd_len;

    filt = (double *)malloc (sizeof (double) * 2 * len_avg);
    s = sqrt (2.0);
    for (i = 0; i < len_avg; ++i)
    {
        filt[i] = wt->wave->lpd[i] / s;
        filt[len_avg + i] = wt->wave->hpd[i] / s;
    }

    for (i = 0; i < len_cA; ++i)
    {
        t = i;
        cA[i] = filt[0] * inp[t];
        cD[i] = filt[len_avg] * inp[t];
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

            cA[i] += filt[l] * inp[t];
            cD[i] += filt[len_avg + l] * inp[t];
        }
    }
    free (filt);
}

static void modwt_direct (wt_object wt, const double *inp)
{
    int i, J, temp_len, iter, M;
    int lenacc;
    double *cA, *cD;

    if (strcmp (wt->ext, "per"))
    {
        throw std::runtime_error ("wavelib error");
    }

    temp_len = wt->siglength;
    J = wt->J;
    wt->length[0] = wt->length[J] = temp_len;
    wt->outlength = wt->length[J + 1] = (J + 1) * temp_len;
    M = 1;
    for (iter = 1; iter < J; ++iter)
    {
        M = 2 * M;
        wt->length[iter] = temp_len;
    }


    cA = (double *)malloc (sizeof (double) * temp_len);
    cD = (double *)malloc (sizeof (double) * temp_len);

    M = 1;

    for (i = 0; i < temp_len; ++i)
    {
        wt->params[i] = inp[i];
    }

    lenacc = wt->outlength;

    for (iter = 0; iter < J; ++iter)
    {
        lenacc -= temp_len;
        if (iter > 0)
        {
            M = 2 * M;
        }

        modwt_per (wt, M, wt->params, cA, temp_len, cD);


        for (i = 0; i < temp_len; ++i)
        {
            wt->params[i] = cA[i];
            wt->params[lenacc + i] = cD[i];
        }
    }

    free (cA);
    free (cD);
}

static void modwt_fft (wt_object wt, const double *inp)
{
    int i, J, temp_len, iter, M, N = 0, len_avg;
    int lenacc;
    double s, tmp1, tmp2;
    fft_data *cA, *cD, *low_pass, *high_pass, *sig;
    int *index;
    fft_object fft_fd = NULL;
    fft_object fft_bd = NULL;

    temp_len = wt->siglength;
    len_avg = wt->wave->lpd_len;
    if (!strcmp (wt->ext, "sym"))
    {
        N = 2 * temp_len;
    }
    else if (!strcmp (wt->ext, "per"))
    {
        N = temp_len;
    }
    J = wt->J;
    wt->modwtsiglength = N;
    wt->length[0] = wt->length[J] = N;
    wt->outlength = wt->length[J + 1] = (J + 1) * N;

    s = sqrt (2.0);
    for (iter = 1; iter < J; ++iter)
    {
        wt->length[iter] = N;
    }

    fft_fd = fft_init (N, 1);
    fft_bd = fft_init (N, -1);

    sig = (fft_data *)malloc (sizeof (fft_data) * N);
    cA = (fft_data *)malloc (sizeof (fft_data) * N);
    cD = (fft_data *)malloc (sizeof (fft_data) * N);
    low_pass = (fft_data *)malloc (sizeof (fft_data) * N);
    high_pass = (fft_data *)malloc (sizeof (fft_data) * N);
    index = (int *)malloc (sizeof (int) * N);


    // N-point FFT of low pass and high pass filters

    // Low Pass Filter

    for (i = 0; i < len_avg; ++i)
    {
        sig[i].re = (fft_type)wt->wave->lpd[i] / s;
        sig[i].im = 0.0;
    }
    for (i = len_avg; i < N; ++i)
    {
        sig[i].re = 0.0;
        sig[i].im = 0.0;
    }


    fft_exec (fft_fd, sig, low_pass);

    // High Pass Filter

    for (i = 0; i < len_avg; ++i)
    {
        sig[i].re = (fft_type)wt->wave->hpd[i] / s;
        sig[i].im = 0.0;
    }
    for (i = len_avg; i < N; ++i)
    {
        sig[i].re = 0.0;
        sig[i].im = 0.0;
    }

    fft_exec (fft_fd, sig, high_pass);

    // symmetric extension
    for (i = 0; i < temp_len; ++i)
    {
        sig[i].re = (fft_type)inp[i];
        sig[i].im = 0.0;
    }
    for (i = temp_len; i < N; ++i)
    {
        sig[i].re = (fft_type)inp[N - i - 1];
        sig[i].im = 0.0;
    }

    // FFT of data

    fft_exec (fft_fd, sig, cA);

    lenacc = wt->outlength;

    M = 1;

    for (iter = 0; iter < J; ++iter)
    {
        lenacc -= N;

        for (i = 0; i < N; ++i)
        {
            index[i] = (M * i) % N;
        }

        for (i = 0; i < N; ++i)
        {
            tmp1 = cA[i].re;
            tmp2 = cA[i].im;
            cA[i].re = low_pass[index[i]].re * tmp1 - low_pass[index[i]].im * tmp2;
            cA[i].im = low_pass[index[i]].re * tmp2 + low_pass[index[i]].im * tmp1;

            cD[i].re = high_pass[index[i]].re * tmp1 - high_pass[index[i]].im * tmp2;
            cD[i].im = high_pass[index[i]].re * tmp2 + high_pass[index[i]].im * tmp1;
        }

        fft_exec (fft_bd, cD, sig);

        for (i = 0; i < N; ++i)
        {
            wt->params[lenacc + i] = sig[i].re / N;
        }

        M *= 2;
    }

    fft_exec (fft_bd, cA, sig);

    for (i = 0; i < N; ++i)
    {
        wt->params[i] = sig[i].re / N;
    }

    free (sig);
    free (cA);
    free (cD);
    free (low_pass);
    free (high_pass);
    free_fft (fft_fd);
    free_fft (fft_bd);
}

void modwt (wt_object wt, const double *inp)
{
    if (!strcmp (wt->cmethod, "direct"))
    {
        modwt_direct (wt, inp);
    }
    else if (!strcmp (wt->cmethod, "fft"))
    {
        modwt_fft (wt, inp);
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}

static void conj_complex (fft_data *x, int N)
{
    int i;

    for (i = 0; i < N; ++i)
    {
        x[i].im *= (-1.0);
    }
}

static void getMODWTRecCoeff (fft_object fft_fd, fft_object fft_bd, fft_data *appx, fft_data *det,
    fft_data *cA, fft_data *cD, int *index, const char *ctype, int level, int J, fft_data *low_pass,
    fft_data *high_pass, int N)
{

    int iter, M, i;
    fft_type tmp1, tmp2;

    M = (int)pow (2.0, (double)level - 1.0);

    if (!strcmp ((ctype), "appx"))
    {
        for (iter = 0; iter < level; ++iter)
        {
            fft_exec (fft_fd, appx, cA);
            fft_exec (fft_fd, det, cD);

            for (i = 0; i < N; ++i)
            {
                index[i] = (M * i) % N;
            }

            for (i = 0; i < N; ++i)
            {
                tmp1 = cA[i].re;
                tmp2 = cA[i].im;
                cA[i].re = low_pass[index[i]].re * tmp1 - low_pass[index[i]].im * tmp2 +
                    high_pass[index[i]].re * cD[i].re - high_pass[index[i]].im * cD[i].im;
                cA[i].im = low_pass[index[i]].re * tmp2 + low_pass[index[i]].im * tmp1 +
                    high_pass[index[i]].re * cD[i].im + high_pass[index[i]].im * cD[i].re;
            }

            fft_exec (fft_bd, cA, appx);

            for (i = 0; i < N; ++i)
            {
                appx[i].re /= N;
                appx[i].im /= N;
            }

            M /= 2;
        }
    }
    else if (!strcmp (ctype, "det"))
    {
        for (iter = 0; iter < level; ++iter)
        {
            fft_exec (fft_fd, appx, cA);
            fft_exec (fft_fd, det, cD);

            for (i = 0; i < N; ++i)
            {
                index[i] = (M * i) % N;
            }

            for (i = 0; i < N; ++i)
            {
                tmp1 = cA[i].re;
                tmp2 = cA[i].im;
                cA[i].re = low_pass[index[i]].re * tmp1 - low_pass[index[i]].im * tmp2 +
                    high_pass[index[i]].re * cD[i].re - high_pass[index[i]].im * cD[i].im;
                cA[i].im = low_pass[index[i]].re * tmp2 + low_pass[index[i]].im * tmp1 +
                    high_pass[index[i]].re * cD[i].im + high_pass[index[i]].im * cD[i].re;
            }

            fft_exec (fft_bd, cA, appx);

            for (i = 0; i < N; ++i)
            {
                appx[i].re /= N;
                appx[i].im /= N;
                det[i].re = 0.0;
                det[i].im = 0.0;
            }

            M /= 2;
        }
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}

double *getMODWTmra (wt_object wt, double *wavecoeffs)
{
    double *mra;
    int i, J, temp_len = 0, iter, M, N, len_avg, lmra;
    int lenacc;
    double s;
    fft_data *cA, *cD, *low_pass, *high_pass, *sig, *ninp;
    int *index;
    fft_object fft_fd = NULL;
    fft_object fft_bd = NULL;

    N = wt->modwtsiglength;
    len_avg = wt->wave->lpd_len;
    if (!strcmp (wt->ext, "sym"))
    {
        temp_len = N / 2;
    }
    else if (!strcmp (wt->ext, "per"))
    {
        temp_len = N;
    }
    J = wt->J;

    s = sqrt (2.0);
    fft_fd = fft_init (N, 1);
    fft_bd = fft_init (N, -1);

    sig = (fft_data *)malloc (sizeof (fft_data) * N);
    cA = (fft_data *)malloc (sizeof (fft_data) * N);
    cD = (fft_data *)malloc (sizeof (fft_data) * N);
    ninp = (fft_data *)malloc (sizeof (fft_data) * N);
    low_pass = (fft_data *)malloc (sizeof (fft_data) * N);
    high_pass = (fft_data *)malloc (sizeof (fft_data) * N);
    index = (int *)malloc (sizeof (int) * N);
    mra = (double *)malloc (sizeof (double) * temp_len * (J + 1));

    // N-point FFT of low pass and high pass filters

    // Low Pass Filter

    for (i = 0; i < len_avg; ++i)
    {
        sig[i].re = (fft_type)wt->wave->lpd[i] / s;
        sig[i].im = 0.0;
    }
    for (i = len_avg; i < N; ++i)
    {
        sig[i].re = 0.0;
        sig[i].im = 0.0;
    }


    fft_exec (fft_fd, sig, low_pass);

    // High Pass Filter

    for (i = 0; i < len_avg; ++i)
    {
        sig[i].re = (fft_type)wt->wave->hpd[i] / s;
        sig[i].im = 0.0;
    }
    for (i = len_avg; i < N; ++i)
    {
        sig[i].re = 0.0;
        sig[i].im = 0.0;
    }

    fft_exec (fft_fd, sig, high_pass);


    // Complex conjugate of the two filters

    conj_complex (low_pass, N);
    conj_complex (high_pass, N);

    M = (int)pow (2.0, (double)J - 1.0);
    lenacc = N;

    //
    for (i = 0; i < N; ++i)
    {
        sig[i].re = (fft_type)wt->output[i];
        sig[i].im = 0.0;
        ninp[i].re = 0.0;
        ninp[i].im = 0.0;
    }

    // Find Approximation MRA

    getMODWTRecCoeff (
        fft_fd, fft_bd, sig, ninp, cA, cD, index, "appx", J, J, low_pass, high_pass, N);

    for (i = 0; i < wt->siglength; ++i)
    {
        mra[i] = sig[i].re;
    }
    lmra = wt->siglength;
    // Find Details MRA

    for (iter = 0; iter < J; ++iter)
    {
        for (i = 0; i < N; ++i)
        {
            sig[i].re = (fft_type)wt->output[lenacc + i];
            sig[i].im = 0.0;
            ninp[i].re = 0.0;
            ninp[i].im = 0.0;
        }

        getMODWTRecCoeff (
            fft_fd, fft_bd, sig, ninp, cA, cD, index, "det", J - iter, J, low_pass, high_pass, N);

        for (i = 0; i < wt->siglength; ++i)
        {
            mra[lmra + i] = sig[i].re;
        }

        lenacc += N;
        lmra += wt->siglength;
    }

    free (ninp);
    free (index);
    free (sig);
    free (cA);
    free (cD);
    free (low_pass);
    free (high_pass);
    free_fft (fft_fd);
    free_fft (fft_bd);

    return mra;
}

void imodwt_fft (wt_object wt, double *oup)
{
    int i, J, temp_len, iter, M, N, len_avg;
    int lenacc;
    double s, tmp1, tmp2;
    fft_data *cA, *cD, *low_pass, *high_pass, *sig;
    int *index;
    fft_object fft_fd = NULL;
    fft_object fft_bd = NULL;

    N = wt->modwtsiglength;
    len_avg = wt->wave->lpd_len;
    if (!strcmp (wt->ext, "sym"))
    {
        temp_len = N / 2;
    }
    else if (!strcmp (wt->ext, "per"))
    {
        temp_len = N;
    }
    J = wt->J;

    s = sqrt (2.0);
    fft_fd = fft_init (N, 1);
    fft_bd = fft_init (N, -1);

    sig = (fft_data *)malloc (sizeof (fft_data) * N);
    cA = (fft_data *)malloc (sizeof (fft_data) * N);
    cD = (fft_data *)malloc (sizeof (fft_data) * N);
    low_pass = (fft_data *)malloc (sizeof (fft_data) * N);
    high_pass = (fft_data *)malloc (sizeof (fft_data) * N);
    index = (int *)malloc (sizeof (int) * N);


    // N-point FFT of low pass and high pass filters

    // Low Pass Filter

    for (i = 0; i < len_avg; ++i)
    {
        sig[i].re = (fft_type)wt->wave->lpd[i] / s;
        sig[i].im = 0.0;
    }
    for (i = len_avg; i < N; ++i)
    {
        sig[i].re = 0.0;
        sig[i].im = 0.0;
    }


    fft_exec (fft_fd, sig, low_pass);

    // High Pass Filter

    for (i = 0; i < len_avg; ++i)
    {
        sig[i].re = (fft_type)wt->wave->hpd[i] / s;
        sig[i].im = 0.0;
    }
    for (i = len_avg; i < N; ++i)
    {
        sig[i].re = 0.0;
        sig[i].im = 0.0;
    }

    fft_exec (fft_fd, sig, high_pass);


    // Complex conjugate of the two filters

    conj_complex (low_pass, N);
    conj_complex (high_pass, N);

    M = (int)pow (2.0, (double)J - 1.0);
    lenacc = N;

    //
    for (i = 0; i < N; ++i)
    {
        sig[i].re = (fft_type)wt->output[i];
        sig[i].im = 0.0;
    }

    for (iter = 0; iter < J; ++iter)
    {
        fft_exec (fft_fd, sig, cA);
        for (i = 0; i < N; ++i)
        {
            sig[i].re = wt->output[lenacc + i];
            sig[i].im = 0.0;
        }
        fft_exec (fft_fd, sig, cD);

        for (i = 0; i < N; ++i)
        {
            index[i] = (M * i) % N;
        }

        for (i = 0; i < N; ++i)
        {
            tmp1 = cA[i].re;
            tmp2 = cA[i].im;
            cA[i].re = low_pass[index[i]].re * tmp1 - low_pass[index[i]].im * tmp2 +
                high_pass[index[i]].re * cD[i].re - high_pass[index[i]].im * cD[i].im;
            cA[i].im = low_pass[index[i]].re * tmp2 + low_pass[index[i]].im * tmp1 +
                high_pass[index[i]].re * cD[i].im + high_pass[index[i]].im * cD[i].re;
        }

        fft_exec (fft_bd, cA, sig);

        for (i = 0; i < N; ++i)
        {
            sig[i].re /= N;
            sig[i].im /= N;
        }
        M /= 2;
        lenacc += N;
    }

    for (i = 0; i < wt->siglength; ++i)
    {
        oup[i] = sig[i].re;
    }

    free (sig);
    free (cA);
    free (cD);
    free (low_pass);
    free (high_pass);
    free_fft (fft_fd);
    free_fft (fft_bd);
}


static void imodwt_per (wt_object wt, int M, double *cA, int len_cA, double *cD, double *X)
{
    int len_avg, i, l, t;
    double s;
    double *filt;
    len_avg = wt->wave->lpd_len;

    filt = (double *)malloc (sizeof (double) * 2 * len_avg);
    s = sqrt (2.0);
    for (i = 0; i < len_avg; ++i)
    {
        filt[i] = wt->wave->lpd[i] / s;
        filt[len_avg + i] = wt->wave->hpd[i] / s;
    }


    for (i = 0; i < len_cA; ++i)
    {
        t = i;
        X[i] = (filt[0] * cA[t]) + (filt[len_avg] * cD[t]);
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

            X[i] += (filt[l] * cA[t]) + (filt[len_avg + l] * cD[t]);
        }
    }
    free (filt);
}

static void imodwt_direct (wt_object wt, double *dwtop)
{
    int N, iter, i, J, j;
    int lenacc, M;
    double *X;

    N = wt->siglength;
    J = wt->J;
    lenacc = N;
    M = (int)pow (2.0, (double)J - 1.0);
    // M = 1;
    X = (double *)malloc (sizeof (double) * N);

    for (i = 0; i < N; ++i)
    {
        dwtop[i] = wt->output[i];
    }

    for (iter = 0; iter < J; ++iter)
    {
        if (iter > 0)
        {
            M = M / 2;
        }
        imodwt_per (wt, M, dwtop, N, wt->params + lenacc, X);
        /*
        for (j = lf - 1; j < N; ++j) {
            dwtop[j - lf + 1] = X[j];
        }
        for (j = 0; j < lf - 1; ++j) {
            dwtop[N - lf + 1 + j] = X[j];
        }
        */
        for (j = 0; j < N; ++j)
        {
            dwtop[j] = X[j];
        }

        lenacc += N;
    }
    free (X);
}

void imodwt (wt_object wt, double *oup)
{
    if (!strcmp (wt->cmethod, "direct"))
    {
        imodwt_direct (wt, oup);
    }
    else if (!strcmp (wt->cmethod, "fft"))
    {
        imodwt_fft (wt, oup);
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}


void setDWTExtension (wt_object wt, const char *extension)
{
    if (!strcmp (extension, "sym"))
    {
        strcpy (wt->ext, "sym");
    }
    else if (!strcmp (extension, "per"))
    {
        strcpy (wt->ext, "per");
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}

void setWTREEExtension (wtree_object wt, const char *extension)
{
    if (!strcmp (extension, "sym"))
    {
        strcpy (wt->ext, "sym");
    }
    else if (!strcmp (extension, "per"))
    {
        strcpy (wt->ext, "per");
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}

void setDWPTExtension (wpt_object wt, const char *extension)
{
    if (!strcmp (extension, "sym"))
    {
        strcpy (wt->ext, "sym");
    }
    else if (!strcmp (extension, "per"))
    {
        strcpy (wt->ext, "per");
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}

void setDWT2Extension (wt2_object wt, const char *extension)
{
    if (!strcmp (wt->method, "dwt"))
    {
        if (!strcmp (extension, "sym"))
        {
            strcpy (wt->ext, "sym");
        }
        else if (!strcmp (extension, "per"))
        {
            strcpy (wt->ext, "per");
        }
        else
        {
            throw std::runtime_error ("wavelib error");
        }
    }
    else if (!strcmp (wt->method, "swt") || !strcmp (wt->method, "modwt"))
    {
        if (!strcmp (extension, "per"))
        {
            strcpy (wt->ext, "per");
        }
        else
        {
            throw std::runtime_error ("wavelib error");
        }
    }
}

void setDWPTEntropy (wpt_object wt, const char *entropy, double eparam)
{
    if (!strcmp (entropy, "shannon"))
    {
        strcpy (wt->entropy, "shannon");
    }
    else if (!strcmp (entropy, "threshold"))
    {
        strcpy (wt->entropy, "threshold");
        wt->eparam = eparam;
    }
    else if (!strcmp (entropy, "norm"))
    {
        strcpy (wt->entropy, "norm");
        wt->eparam = eparam;
    }
    else if (!strcmp (entropy, "logenergy") || !strcmp (entropy, "log energy") ||
        !strcmp (entropy, "energy"))
    {
        strcpy (wt->entropy, "logenergy");
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}

void setWTConv (wt_object wt, const char *cmethod)
{
    if (!strcmp (cmethod, "fft") || !strcmp (cmethod, "FFT"))
    {
        strcpy (wt->cmethod, "fft");
    }
    else if (!strcmp (cmethod, "direct"))
    {
        strcpy (wt->cmethod, "direct");
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }
}

double *dwt2 (wt2_object wt, double *inp)
{
    double *wavecoeff = NULL;
    int i, J, iter, N, lp, rows_n, cols_n, rows_i, cols_i;
    int ir, ic, istride, ostride;
    int aLL, aLH, aHL, aHH, cdim, clen;
    double *orig, *lp_dn1, *hp_dn1;
    J = wt->J;
    wt->outlength = 0;

    rows_n = wt->rows;
    cols_n = wt->cols;
    lp = wt->wave->lpd_len;
    clen = J * 3;
    if (!strcmp (wt->ext, "per"))
    {
        i = 2 * J;
        while (i > 0)
        {
            rows_n = (int)ceil ((double)rows_n / 2.0);
            cols_n = (int)ceil ((double)cols_n / 2.0);
            wt->dimensions[i - 1] = cols_n;
            wt->dimensions[i - 2] = rows_n;
            wt->outlength += (rows_n * cols_n) * 3;
            i = i - 2;
        }
        wt->outlength += (rows_n * cols_n);
        N = wt->outlength;
        wavecoeff = (double *)calloc (wt->outlength, sizeof (double));

        orig = inp;
        ir = wt->rows;
        ic = wt->cols;
        cols_i = wt->dimensions[2 * J - 1];

        lp_dn1 = (double *)malloc (sizeof (double) * ir * cols_i);
        hp_dn1 = (double *)malloc (sizeof (double) * ir * cols_i);

        for (iter = 0; iter < J; ++iter)
        {
            rows_i = wt->dimensions[2 * J - 2 * iter - 2];
            cols_i = wt->dimensions[2 * J - 2 * iter - 1];
            istride = 1;
            ostride = 1;
            cdim = rows_i * cols_i;
            // Row filtering and column subsampling
            for (i = 0; i < ir; ++i)
            {
                dwt_per_stride (orig + i * ic, ic, wt->wave->lpd, wt->wave->hpd, lp,
                    lp_dn1 + i * cols_i, cols_i, hp_dn1 + i * cols_i, istride, ostride);
            }

            // Column Filtering and Row subsampling
            aHH = N - cdim;
            wt->coeffaccess[clen] = aHH;
            aHL = aHH - cdim;
            wt->coeffaccess[clen - 1] = aHL;
            aLH = aHL - cdim;
            wt->coeffaccess[clen - 2] = aLH;
            aLL = aLH - cdim;

            N -= 3 * cdim;
            ic = cols_i;
            istride = ic;
            ostride = ic;

            for (i = 0; i < ic; ++i)
            {
                dwt_per_stride (lp_dn1 + i, ir, wt->wave->lpd, wt->wave->hpd, lp,
                    wavecoeff + aLL + i, rows_i, wavecoeff + aLH + i, istride, ostride);
            }


            for (i = 0; i < ic; ++i)
            {
                dwt_per_stride (hp_dn1 + i, ir, wt->wave->lpd, wt->wave->hpd, lp,
                    wavecoeff + aHL + i, rows_i, wavecoeff + aHH + i, istride, ostride);
            }

            ir = rows_i;
            orig = wavecoeff + aLL;
            clen -= 3;
        }
        wt->coeffaccess[0] = 0;
        free (lp_dn1);
        free (hp_dn1);
    }
    else if (!strcmp (wt->ext, "sym"))
    {
        i = 2 * J;
        while (i > 0)
        {
            rows_n += lp - 2;
            cols_n += lp - 2;
            rows_n = (int)ceil ((double)rows_n / 2.0);
            cols_n = (int)ceil ((double)cols_n / 2.0);
            wt->dimensions[i - 1] = cols_n;
            wt->dimensions[i - 2] = rows_n;
            wt->outlength += (rows_n * cols_n) * 3;
            i = i - 2;
        }
        wt->outlength += (rows_n * cols_n);
        N = wt->outlength;
        wavecoeff = (double *)calloc (wt->outlength, sizeof (double));

        orig = inp;
        ir = wt->rows;
        ic = wt->cols;
        cols_i = wt->dimensions[2 * J - 1];

        lp_dn1 = (double *)malloc (sizeof (double) * ir * cols_i);
        hp_dn1 = (double *)malloc (sizeof (double) * ir * cols_i);

        for (iter = 0; iter < J; ++iter)
        {
            rows_i = wt->dimensions[2 * J - 2 * iter - 2];
            cols_i = wt->dimensions[2 * J - 2 * iter - 1];
            istride = 1;
            ostride = 1;
            cdim = rows_i * cols_i;
            // Row filtering and column subsampling
            for (i = 0; i < ir; ++i)
            {
                dwt_sym_stride (orig + i * ic, ic, wt->wave->lpd, wt->wave->hpd, lp,
                    lp_dn1 + i * cols_i, cols_i, hp_dn1 + i * cols_i, istride, ostride);
            }

            // Column Filtering and Row subsampling
            aHH = N - cdim;
            wt->coeffaccess[clen] = aHH;
            aHL = aHH - cdim;
            wt->coeffaccess[clen - 1] = aHL;
            aLH = aHL - cdim;
            wt->coeffaccess[clen - 2] = aLH;
            aLL = aLH - cdim;
            N -= 3 * cdim;
            ic = cols_i;
            istride = ic;
            ostride = ic;

            for (i = 0; i < ic; ++i)
            {
                dwt_sym_stride (lp_dn1 + i, ir, wt->wave->lpd, wt->wave->hpd, lp,
                    wavecoeff + aLL + i, rows_i, wavecoeff + aLH + i, istride, ostride);
            }

            for (i = 0; i < ic; ++i)
            {
                dwt_sym_stride (hp_dn1 + i, ir, wt->wave->lpd, wt->wave->hpd, lp,
                    wavecoeff + aHL + i, rows_i, wavecoeff + aHH + i, istride, ostride);
            }

            ir = rows_i;
            orig = wavecoeff + aLL;
            clen -= 3;
        }
        wt->coeffaccess[0] = 0;
        free (lp_dn1);
        free (hp_dn1);
    }

    return wavecoeff;
}

void idwt2 (wt2_object wt, double *wavecoeff, double *oup)
{
    int i, k, rows, cols, N, ir, ic, lf, dim1, dim2;
    int istride, ostride, iter, J;
    int aLL, aLH, aHL, aHH;
    double *cL, *cH, *X_lp, *orig;

    rows = wt->rows;
    cols = wt->cols;
    J = wt->J;
    double *out = NULL;

    if (!strcmp (wt->ext, "per"))
    {
        N = rows > cols ? 2 * rows : 2 * cols;
        lf = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;

        i = J;
        dim1 = wt->dimensions[0];
        dim2 = wt->dimensions[1];
        k = 0;
        while (i > 0)
        {
            k += 1;
            dim1 *= 2;
            dim2 *= 2;
            i--;
        }


        X_lp = (double *)malloc (sizeof (double) * (N + 2 * lf - 1));
        cL = (double *)calloc (dim1 * dim2, sizeof (double));
        cH = (double *)calloc (dim1 * dim2, sizeof (double));
        out = (double *)calloc (dim1 * dim2, sizeof (double));
        aLL = wt->coeffaccess[0];
        orig = wavecoeff + aLL;
        for (iter = 0; iter < J; ++iter)
        {
            ir = wt->dimensions[2 * iter];
            ic = wt->dimensions[2 * iter + 1];
            istride = ic;
            ostride = 1;
            aLH = wt->coeffaccess[iter * 3 + 1];
            aHL = wt->coeffaccess[iter * 3 + 2];
            aHH = wt->coeffaccess[iter * 3 + 3];
            for (i = 0; i < ic; ++i)
            {
                idwt_per_stride (orig + i, ir, wavecoeff + aLH + i, wt->wave->lpr, wt->wave->hpr,
                    lf, X_lp, istride, ostride);

                for (k = lf / 2 - 1; k < 2 * ir + lf / 2 - 1; ++k)
                {
                    cL[(k - lf / 2 + 1) * ic + i] = X_lp[k];
                }

                idwt_per_stride (wavecoeff + aHL + i, ir, wavecoeff + aHH + i, wt->wave->lpr,
                    wt->wave->hpr, lf, X_lp, istride, ostride);

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
                idwt_per_stride (cL + i * ic, ic, cH + i * ic, wt->wave->lpr, wt->wave->hpr, lf,
                    X_lp, istride, ostride);

                for (k = lf / 2 - 1; k < 2 * ic + lf / 2 - 1; ++k)
                {
                    out[(k - lf / 2 + 1) + i * ic * 2] = X_lp[k];
                }
            }
            ic *= 2;
            if (iter == J - 1)
            {
                for (i = 0; i < wt->rows; ++i)
                {
                    for (k = 0; k < wt->cols; ++k)
                    {
                        oup[k + i * wt->cols] = out[k + i * ic];
                    }
                }
            }
            else
            {
                for (i = 0; i < wt->dimensions[2 * (iter + 1)]; ++i)
                {
                    for (k = 0; k < wt->dimensions[2 * (iter + 1) + 1]; ++k)
                    {
                        oup[k + i * wt->dimensions[2 * (iter + 1) + 1]] = out[k + i * ic];
                    }
                }
            }


            orig = oup;
        }
        free (X_lp);
        free (cL);
        free (cH);
    }
    else if (!strcmp (wt->ext, "sym"))
    {
        N = rows > cols ? 2 * rows - 1 : 2 * cols - 1;
        lf = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;

        i = J;
        dim1 = wt->dimensions[0];
        dim2 = wt->dimensions[1];
        k = 0;
        while (i > 0)
        {
            k += 1;
            dim1 *= 2;
            dim2 *= 2;
            i--;
        }


        X_lp = (double *)malloc (sizeof (double) * (N + 2 * lf - 1));
        cL = (double *)calloc (dim1 * dim2, sizeof (double));
        cH = (double *)calloc (dim1 * dim2, sizeof (double));
        out = (double *)calloc (dim1 * dim2, sizeof (double));
        aLL = wt->coeffaccess[0];
        orig = wavecoeff + aLL;
        for (iter = 0; iter < J; ++iter)
        {
            ir = wt->dimensions[2 * iter];
            ic = wt->dimensions[2 * iter + 1];
            istride = ic;
            ostride = 1;
            aLH = wt->coeffaccess[iter * 3 + 1];
            aHL = wt->coeffaccess[iter * 3 + 2];
            aHH = wt->coeffaccess[iter * 3 + 3];
            for (i = 0; i < ic; ++i)
            {
                idwt_sym_stride (orig + i, ir, wavecoeff + aLH + i, wt->wave->lpr, wt->wave->hpr,
                    lf, X_lp, istride, ostride);

                for (k = lf - 2; k < 2 * ir; ++k)
                {
                    cL[(k - lf + 2) * ic + i] = X_lp[k];
                }

                idwt_sym_stride (wavecoeff + aHL + i, ir, wavecoeff + aHH + i, wt->wave->lpr,
                    wt->wave->hpr, lf, X_lp, istride, ostride);

                for (k = lf - 2; k < 2 * ir; ++k)
                {
                    cH[(k - lf + 2) * ic + i] = X_lp[k];
                }
            }

            ir *= 2;
            istride = 1;
            ostride = 1;

            for (i = 0; i < ir; ++i)
            {
                idwt_sym_stride (cL + i * ic, ic, cH + i * ic, wt->wave->lpr, wt->wave->hpr, lf,
                    X_lp, istride, ostride);

                for (k = lf - 2; k < 2 * ic; ++k)
                {
                    out[(k - lf + 2) + i * ic * 2] = X_lp[k];
                }
            }
            ic *= 2;
            if (iter == J - 1)
            {
                for (i = 0; i < wt->rows; ++i)
                {
                    for (k = 0; k < wt->cols; ++k)
                    {
                        oup[k + i * wt->cols] = out[k + i * ic];
                    }
                }
            }
            else
            {
                for (i = 0; i < wt->dimensions[2 * (iter + 1)]; ++i)
                {
                    for (k = 0; k < wt->dimensions[2 * (iter + 1) + 1]; ++k)
                    {
                        oup[k + i * wt->dimensions[2 * (iter + 1) + 1]] = out[k + i * ic];
                    }
                }
            }


            orig = oup;
        }
        free (X_lp);
        free (cL);
        free (cH);
    }

    free (out);
}

double *swt2 (wt2_object wt, double *inp)
{
    double *wavecoeff;
    int i, J, iter, M, N, lp, rows_n, cols_n, rows_i, cols_i;
    int ir, ic, istride, ostride;
    int aLL, aLH, aHL, aHH, cdim, clen;
    double *orig, *lp_dn1, *hp_dn1;

    J = wt->J;
    M = 1;
    wt->outlength = 0;

    rows_n = wt->rows;
    cols_n = wt->cols;
    lp = wt->wave->lpd_len;
    clen = J * 3;

    i = 2 * J;
    while (i > 0)
    {
        wt->dimensions[i - 1] = cols_n;
        wt->dimensions[i - 2] = rows_n;
        wt->outlength += (rows_n * cols_n) * 3;
        i = i - 2;
    }
    wt->outlength += (rows_n * cols_n);
    N = wt->outlength;
    wavecoeff = (double *)calloc (wt->outlength, sizeof (double));

    orig = inp;
    ir = wt->rows;
    ic = wt->cols;
    cols_i = wt->dimensions[2 * J - 1];

    lp_dn1 = (double *)malloc (sizeof (double) * ir * cols_i);
    hp_dn1 = (double *)malloc (sizeof (double) * ir * cols_i);

    for (iter = 0; iter < J; ++iter)
    {
        if (iter > 0)
        {
            M = 2 * M;
        }
        rows_i = wt->dimensions[2 * J - 2 * iter - 2];
        cols_i = wt->dimensions[2 * J - 2 * iter - 1];
        istride = 1;
        ostride = 1;
        cdim = rows_i * cols_i;
        // Row filtering and column subsampling
        for (i = 0; i < ir; ++i)
        {
            swt_per_stride (M, orig + i * ic, ic, wt->wave->lpd, wt->wave->hpd, lp,
                lp_dn1 + i * cols_i, cols_i, hp_dn1 + i * cols_i, istride, ostride);
        }
        // Column Filtering and Row subsampling
        aHH = N - cdim;
        wt->coeffaccess[clen] = aHH;
        aHL = aHH - cdim;
        wt->coeffaccess[clen - 1] = aHL;
        aLH = aHL - cdim;
        wt->coeffaccess[clen - 2] = aLH;
        aLL = aLH - cdim;

        N -= 3 * cdim;
        ic = cols_i;
        istride = ic;
        ostride = ic;
        for (i = 0; i < ic; ++i)
        {
            swt_per_stride (M, lp_dn1 + i, ir, wt->wave->lpd, wt->wave->hpd, lp,
                wavecoeff + aLL + i, rows_i, wavecoeff + aLH + i, istride, ostride);
        }

        for (i = 0; i < ic; ++i)
        {
            swt_per_stride (M, hp_dn1 + i, ir, wt->wave->lpd, wt->wave->hpd, lp,
                wavecoeff + aHL + i, rows_i, wavecoeff + aHH + i, istride, ostride);
        }

        ir = rows_i;
        orig = wavecoeff + aLL;
        clen -= 3;
    }
    wt->coeffaccess[0] = 0;
    free (lp_dn1);
    free (hp_dn1);

    return wavecoeff;
}

void iswt2 (wt2_object wt, double *wavecoeffs, double *oup)
{
    int i, k, iter, it2, it3, J, M, rows, cols, lf, ir, ic, k1, i1;
    double *A, *H, *V, *D, *oup1, *oup2;
    int aLL, aLH, aHL, aHH, shift;
    J = wt->J;
    rows = wt->rows;
    cols = wt->cols;
    lf = wt->wave->lpd_len;
    A = (double *)calloc ((rows + lf) * (cols + lf), sizeof (double));
    H = (double *)calloc ((rows + lf) * (cols + lf), sizeof (double));
    V = (double *)calloc ((rows + lf) * (cols + lf), sizeof (double));
    D = (double *)calloc ((rows + lf) * (cols + lf), sizeof (double));
    oup1 = (double *)calloc ((rows + lf) * (cols + lf), sizeof (double));
    oup2 = (double *)calloc ((rows + lf) * (cols + lf), sizeof (double));

    aLL = wt->coeffaccess[0];

    for (i = 0; i < rows; ++i)
    {
        for (k = 0; k < cols; ++k)
        {
            oup[i * cols + k] = wavecoeffs[aLL + i * cols + k];
        }
    }

    for (iter = J; iter > 0; iter--)
    {
        aLH = wt->coeffaccess[(J - iter) * 3 + 1];
        aHL = wt->coeffaccess[(J - iter) * 3 + 2];
        aHH = wt->coeffaccess[(J - iter) * 3 + 3];
        M = (int)pow (2.0, (double)iter - 1);

        for (it2 = 0; it2 < M; ++it2)
        {
            ir = 0;
            ic = 0;
            it3 = 0;
            // oup1
            for (i = it2; i < rows; i += 2 * M)
            {
                ic = 0;
                for (k = it2; k < cols; k += 2 * M)
                {
                    A[it3] = oup[i * cols + k];
                    H[it3] = wavecoeffs[aLH + i * cols + k];
                    V[it3] = wavecoeffs[aHL + i * cols + k];
                    D[it3] = wavecoeffs[aHH + i * cols + k];
                    it3++;
                    ic++;
                }
                ir++;
            }
            shift = 0;
            idwt2_shift (
                shift, ir, ic, wt->wave->lpr, wt->wave->hpr, wt->wave->lpd_len, A, H, V, D, oup1);
            // oup2
            ir = 0;
            ic = 0;
            it3 = 0;
            for (i = it2 + M; i < rows; i += 2 * M)
            {
                ic = 0;
                for (k = it2 + M; k < cols; k += 2 * M)
                {
                    A[it3] = oup[i * cols + k];
                    H[it3] = wavecoeffs[aLH + i * cols + k];
                    V[it3] = wavecoeffs[aHL + i * cols + k];
                    D[it3] = wavecoeffs[aHH + i * cols + k];
                    it3++;
                    ic++;
                }
                ir++;
            }
            shift = -1;
            idwt2_shift (
                shift, ir, ic, wt->wave->lpr, wt->wave->hpr, wt->wave->lpd_len, A, H, V, D, oup2);
            // Shift oup1 and oup2. Then add them to get A.
            i1 = 0;
            for (i = it2; i < rows; i += M)
            {
                k1 = 0;
                for (k = it2; k < cols; k += M)
                {
                    oup[i * cols + k] = 0.5 * (oup1[i1 * 2 * ic + k1] + oup2[i1 * 2 * ic + k1]);
                    k1++;
                }
                i1++;
            }
        }
    }

    free (A);
    free (H);
    free (V);
    free (D);
    free (oup1);
    free (oup2);
}

double *modwt2 (wt2_object wt, double *inp)
{
    double *wavecoeff;
    int i, J, iter, M, N, lp, rows_n, cols_n, rows_i, cols_i;
    int ir, ic, istride, ostride;
    int aLL, aLH, aHL, aHH, cdim, clen;
    double *orig, *lp_dn1, *hp_dn1, *filt;
    double s;

    J = wt->J;
    M = 1;
    wt->outlength = 0;

    rows_n = wt->rows;
    cols_n = wt->cols;
    lp = wt->wave->lpd_len;
    clen = J * 3;

    i = 2 * J;
    while (i > 0)
    {
        wt->dimensions[i - 1] = cols_n;
        wt->dimensions[i - 2] = rows_n;
        wt->outlength += (rows_n * cols_n) * 3;
        i = i - 2;
    }
    wt->outlength += (rows_n * cols_n);
    N = wt->outlength;
    wavecoeff = (double *)calloc (wt->outlength, sizeof (double));
    filt = (double *)malloc (sizeof (double) * 2 * lp);
    s = sqrt (2.0);
    for (i = 0; i < lp; ++i)
    {
        filt[i] = wt->wave->lpd[i] / s;
        filt[lp + i] = wt->wave->hpd[i] / s;
    }

    orig = inp;
    ir = wt->rows;
    ic = wt->cols;
    cols_i = wt->dimensions[2 * J - 1];

    lp_dn1 = (double *)malloc (sizeof (double) * ir * cols_i);
    hp_dn1 = (double *)malloc (sizeof (double) * ir * cols_i);

    for (iter = 0; iter < J; ++iter)
    {
        if (iter > 0)
        {
            M = 2 * M;
        }
        rows_i = wt->dimensions[2 * J - 2 * iter - 2];
        cols_i = wt->dimensions[2 * J - 2 * iter - 1];
        istride = 1;
        ostride = 1;
        cdim = rows_i * cols_i;
        // Row filtering and column subsampling
        for (i = 0; i < ir; ++i)
        {
            modwt_per_stride (M, orig + i * ic, ic, filt, lp, lp_dn1 + i * cols_i, cols_i,
                hp_dn1 + i * cols_i, istride, ostride);
        }
        // Column Filtering and Row subsampling
        aHH = N - cdim;
        wt->coeffaccess[clen] = aHH;
        aHL = aHH - cdim;
        wt->coeffaccess[clen - 1] = aHL;
        aLH = aHL - cdim;
        wt->coeffaccess[clen - 2] = aLH;
        aLL = aLH - cdim;
        N -= 3 * cdim;
        ic = cols_i;
        istride = ic;
        ostride = ic;
        for (i = 0; i < ic; ++i)
        {
            modwt_per_stride (M, lp_dn1 + i, ir, filt, lp, wavecoeff + aLL + i, rows_i,
                wavecoeff + aLH + i, istride, ostride);
        }


        for (i = 0; i < ic; ++i)
        {
            modwt_per_stride (M, hp_dn1 + i, ir, filt, lp, wavecoeff + aHL + i, rows_i,
                wavecoeff + aHH + i, istride, ostride);
        }


        ir = rows_i;
        orig = wavecoeff + aLL;
        clen -= 3;
    }
    wt->coeffaccess[0] = 0;
    free (lp_dn1);
    free (hp_dn1);
    free (filt);
    return wavecoeff;
}

void imodwt2 (wt2_object wt, double *wavecoeff, double *oup)
{
    int i, rows, cols, M, N, ir, ic, lf;
    int istride, ostride, iter, J;
    int aLL, aLH, aHL, aHH;
    double *cL, *cH, *orig, *filt;
    double s;

    rows = wt->rows;
    cols = wt->cols;
    J = wt->J;


    M = (int)pow (2.0, (double)J - 1.0);
    N = rows > cols ? rows : cols;
    lf = (wt->wave->lpr_len + wt->wave->hpr_len) / 2;

    filt = (double *)malloc (sizeof (double) * 2 * lf);
    s = sqrt (2.0);
    for (i = 0; i < lf; ++i)
    {
        filt[i] = wt->wave->lpd[i] / s;
        filt[lf + i] = wt->wave->hpd[i] / s;
    }


    cL = (double *)calloc (rows * cols, sizeof (double));
    cH = (double *)calloc (rows * cols, sizeof (double));
    aLL = wt->coeffaccess[0];
    orig = wavecoeff + aLL;
    for (iter = 0; iter < J; ++iter)
    {
        if (iter > 0)
        {
            M = M / 2;
        }
        ir = wt->dimensions[2 * iter];
        ic = wt->dimensions[2 * iter + 1];
        istride = ic;
        ostride = ic;
        aLH = wt->coeffaccess[iter * 3 + 1];
        aHL = wt->coeffaccess[iter * 3 + 2];
        aHH = wt->coeffaccess[iter * 3 + 3];
        for (i = 0; i < ic; ++i)
        {
            imodwt_per_stride (
                M, orig + i, ir, wavecoeff + aLH + i, filt, lf, cL + i, istride, ostride);

            imodwt_per_stride (M, wavecoeff + aHL + i, ir, wavecoeff + aHH + i, filt, lf, cH + i,
                istride, ostride);
        }

        istride = 1;
        ostride = 1;

        for (i = 0; i < ir; ++i)
        {
            imodwt_per_stride (
                M, cL + i * ic, ic, cH + i * ic, filt, lf, oup + i * ic, istride, ostride);
        }

        orig = oup;
    }

    free (cL);
    free (cH);
    free (filt);
}

double *getWT2Coeffs (wt2_object wt, double *wcoeffs, int level, char *type, int *rows, int *cols)
{
    int J, iter, t;
    double *ptr;
    J = wt->J;
    // Error Check

    if (level > J || level < 1)
    {
        throw std::runtime_error ("wavelib error");
    }

    if (!strcmp (type, "A") && level != J)
    {
        throw std::runtime_error ("wavelib error");
    }

    if (!strcmp (type, "A"))
    {
        t = 0;
        iter = t;
    }
    else if (!strcmp (type, "H"))
    {
        t = 1;
        iter = t;
    }
    else if (!strcmp (type, "V"))
    {
        t = 2;
        iter = t;
    }
    else if (!strcmp (type, "D"))
    {
        t = 3;
        iter = t;
    }
    else
    {
        throw std::runtime_error ("wavelib error");
    }

    iter += (J - level) * 3;

    ptr = wcoeffs + wt->coeffaccess[iter];
    *rows = wt->dimensions[2 * (J - level)];
    *cols = wt->dimensions[2 * (J - level) + 1];

    return ptr;
}

void dispWT2Coeffs (double *A, int row, int col)
{
    int i, j;
    printf ("\n MATRIX Order : %d X %d \n \n", row, col);

    for (i = 0; i < row; i++)
    {
        printf ("R%d: ", i);
        for (j = 0; j < col; j++)
        {
            printf ("%g ", A[i * col + j]);
        }
        printf (":R%d \n", i);
    }
}

void wave_summary (wave_object obj)
{
    int i, N;
    N = obj->filtlength;
    printf ("\n");
    printf ("Wavelet Name : %s \n", obj->wname);
    printf ("\n");
    printf ("Wavelet Filters \n\n");
    printf ("lpd : [");
    for (i = 0; i < N - 1; ++i)
    {
        printf ("%g,", obj->lpd[i]);
    }
    printf ("%g", obj->lpd[N - 1]);
    printf ("] \n\n");
    printf ("hpd : [");
    for (i = 0; i < N - 1; ++i)
    {
        printf ("%g,", obj->hpd[i]);
    }
    printf ("%g", obj->hpd[N - 1]);
    printf ("] \n\n");
    printf ("lpr : [");
    for (i = 0; i < N - 1; ++i)
    {
        printf ("%g,", obj->lpr[i]);
    }
    printf ("%g", obj->lpr[N - 1]);
    printf ("] \n\n");
    printf ("hpr : [");
    for (i = 0; i < N - 1; ++i)
    {
        printf ("%g,", obj->hpr[i]);
    }
    printf ("%g", obj->hpr[N - 1]);
    printf ("] \n\n");
}

void wt_summary (wt_object wt)
{
    int i;
    int J, t;
    J = wt->J;
    wave_summary (wt->wave);
    printf ("\n");
    printf ("Wavelet Transform : %s \n", wt->method);
    printf ("\n");
    printf ("Signal Extension : %s \n", wt->ext);
    printf ("\n");
    printf ("Convolutional Method : %s \n", wt->cmethod);
    printf ("\n");
    printf ("Number of Decomposition Levels %d \n", wt->J);
    printf ("\n");
    printf ("Length of Input Signal %d \n", wt->siglength);
    printf ("\n");
    printf ("Length of WT Output Vector %d \n", wt->outlength);
    printf ("\n");
    printf ("Wavelet Coefficients are contained in vector : %s \n", "output");
    printf ("\n");
    printf ("Approximation Coefficients \n");
    printf ("Level %d Access : output[%d] Length : %d \n", J, 0, wt->length[0]);
    printf ("\n");
    printf ("Detail Coefficients \n");
    t = wt->length[0];
    for (i = 0; i < J; ++i)
    {
        printf ("Level %d Access : output[%d] Length : %d \n", J - i, t, wt->length[i + 1]);
        t += wt->length[i + 1];
    }
    printf ("\n");
}

void wtree_summary (wtree_object wt)
{
    int i, k, p2;
    int J, t;
    J = wt->J;
    wave_summary (wt->wave);
    printf ("\n");
    printf ("Wavelet Transform : %s \n", wt->method);
    printf ("\n");
    printf ("Signal Extension : %s \n", wt->ext);
    printf ("\n");
    printf ("Number of Decomposition Levels %d \n", wt->J);
    printf ("\n");
    printf ("Length of Input Signal %d \n", wt->siglength);
    printf ("\n");
    printf ("Length of WT Output Vector %d \n", wt->outlength);
    printf ("\n");
    printf ("Wavelet Coefficients are contained in vector : %s \n", "output");
    printf ("\n");
    printf ("Coefficients Access \n");
    t = 0;
    p2 = 2;
    for (i = 0; i < J; ++i)
    {
        for (k = 0; k < p2; ++k)
        {
            printf ("Node %d %d Access : output[%d] Length : %d \n", i + 1, k, wt->nodelength[t],
                wt->length[J - i]);
            t++;
        }
        p2 *= 2;
    }
    printf ("\n");
}

void wpt_summary (wpt_object wt)
{
    int i, k, p2;
    int J, it1, it2;
    J = wt->J;
    wave_summary (wt->wave);
    printf ("\n");
    printf ("Signal Extension : %s \n", wt->ext);
    printf ("\n");
    printf ("Entropy : %s \n", wt->entropy);
    printf ("\n");
    printf ("Number of Decomposition Levels %d \n", wt->J);
    printf ("\n");
    printf ("Number of Active Nodes %d \n", wt->nodes);
    printf ("\n");
    printf ("Length of Input Signal %d \n", wt->siglength);
    printf ("\n");
    printf ("Length of WT Output Vector %d \n", wt->outlength);
    printf ("\n");
    printf ("Wavelet Coefficients are contained in vector : %s \n", "output");
    printf ("\n");
    printf ("Coefficients Access \n");
    it1 = 1;
    it2 = 0;
    for (i = 0; i < J; ++i)
    {
        it1 += ipow2 (i + 1);
    }
    for (i = J; i > 0; --i)
    {
        p2 = ipow2 (i);
        it1 -= p2;
        for (k = 0; k < p2; ++k)
        {
            if (wt->basisvector[it1 + k] == 1)
            {
                printf ("Node %d %d Access : output[%d] Length : %d \n", i, k, it2,
                    wt->length[J - i + 1]);
                it2 += wt->length[J - i + 1];
            }
        }
    }

    printf ("\n");
}

void cwt_summary (cwt_object wt)
{

    printf ("\n");
    printf ("Wavelet : %s Parameter %lf \n", wt->wave, wt->m);
    printf ("\n");
    printf ("Length of Input Signal : %d \n", wt->siglength);
    printf ("\n");
    printf ("Sampling Rate : %g \n", wt->dt);
    printf ("\n");
    printf ("Total Number of Scales : %d \n", wt->J);
    printf ("\n");
    printf ("Smallest Scale (s0) : %lf \n", wt->s0);
    printf ("\n");
    printf ("Separation Between Scales (dj) %lf \n", wt->dj);
    printf ("\n");
    printf ("Scale Type %s \n", wt->type);
    printf ("\n");
    printf ("Complex CWT Output Vector is of size %d * %d stored in Row Major format \n", wt->J,
        wt->siglength);
    printf ("\n");
    printf ("The ith real value can be accessed using wt->output[i].re and imaginary value by "
            "wt->output[i].im \n");
    printf ("\n");
}

void wt2_summary (wt2_object wt)
{
    int i;
    int J, t, rows, cols, vsize;
    J = wt->J;
    wave_summary (wt->wave);
    printf ("\n");
    printf ("Wavelet Transform : %s \n", wt->method);
    printf ("\n");
    printf ("Signal Extension : %s \n", wt->ext);
    printf ("\n");
    printf ("Number of Decomposition Levels %d \n", wt->J);
    printf ("\n");
    printf ("Input Signal Rows %d \n", wt->rows);
    printf ("\n");
    printf ("Input Signal Cols %d \n", wt->cols);
    printf ("\n");
    printf ("Length of Wavelet Coefficients Vector %d \n", wt->outlength);
    printf ("\n");
    t = 0;
    for (i = J; i > 0; --i)
    {
        rows = wt->dimensions[2 * (J - i)];
        cols = wt->dimensions[2 * (J - i) + 1];
        vsize = rows * cols;
        printf ("Level %d Decomposition Rows :%d Columns:%d Vector Size (Rows*Cols):%d \n", i, rows,
            cols, vsize);
        printf ("Access Row values stored at wt->dimensions[%d]\n", 2 * (J - i));
        printf ("Access Column values stored at wt->dimensions[%d]\n\n", 2 * (J - i) + 1);

        if (i == J)
        {
            printf (
                "Approximation Coefficients access at wt->coeffaccess[%d]=%d, Vector size:%d \n", t,
                wt->coeffaccess[t], vsize);
        }

        t += 1;
        printf ("Horizontal Coefficients access at wt->coeffaccess[%d]=%d, Vector size:%d \n", t,
            wt->coeffaccess[t], vsize);
        t += 1;
        printf ("Vertical Coefficients access at wt->coeffaccess[%d]=%d, Vector size:%d \n", t,
            wt->coeffaccess[t], vsize);
        t += 1;
        printf ("Diagonal Coefficients access at wt->coeffaccess[%d]=%d, Vector size:%d \n\n", t,
            wt->coeffaccess[t], vsize);
    }
}

void wave_free (wave_object object)
{
    free (object);
}

void wt_free (wt_object object)
{
    free (object);
}

void wtree_free (wtree_object object)
{
    free (object);
}

void wpt_free (wpt_object object)
{
    free (object);
}

void cwt_free (cwt_object object)
{
    free (object);
}

void wt2_free (wt2_object wt)
{
    free (wt);
}
