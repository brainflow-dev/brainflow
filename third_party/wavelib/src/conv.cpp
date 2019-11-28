/*
 * conv.c
 *
 *  Created on: May 1, 2013
 *      Author: Rafat Hussain
 */

#include "conv.h"

int factorf (int M)
{
    int N;
    N = M;
    while (N % 7 == 0)
    {
        N = N / 7;
    }
    while (N % 3 == 0)
    {
        N = N / 3;
    }
    while (N % 5 == 0)
    {
        N = N / 5;
    }
    while (N % 2 == 0)
    {
        N = N / 2;
    }

    return N;
}


int findnext (int M)
{
    int N;
    N = M;

    while (factorf (N) != 1)
    {
        ++N;
    }

    return N;
}

int findnexte (int M)
{
    int N;
    N = M;

    while (factorf (N) != 1 || N % 2 != 0)
    {
        ++N;
    }

    return N;
}


conv_object conv_init (int N, int L)
{

    conv_object obj = NULL;
    int conv_len;
    conv_len = N + L - 1;

    obj = (conv_object)malloc (sizeof (struct conv_set));

    // obj->clen = npow2(conv_len);
    // obj->clen = conv_len;
    obj->clen = findnexte (conv_len);
    obj->ilen1 = N;
    obj->ilen2 = L;

    obj->fobj = fft_real_init (obj->clen, 1);
    obj->iobj = fft_real_init (obj->clen, -1);

    return obj;
}

void conv_directx (fft_type *inp1, int N, fft_type *inp2, int L, fft_type *oup)
{
    int M, k, n;

    M = N + L - 1;

    for (k = 0; k < M; ++k)
    {
        oup[k] = 0.0;
        for (n = 0; n < N; ++n)
        {
            if ((k - n) >= 0 && (k - n) < L)
            {
                oup[k] += inp1[n] * inp2[k - n];
            }
        }
    }
}

void conv_direct (fft_type *inp1, int N, fft_type *inp2, int L, fft_type *oup)
{

    int M, k, m, i;
    fft_type t1, tmin;

    M = N + L - 1;
    i = 0;

    if (N >= L)
    {

        for (k = 0; k < L; k++)
        {
            oup[k] = 0.0;
            for (m = 0; m <= k; m++)
            {
                oup[k] += inp1[m] * inp2[k - m];
            }
        }

        for (k = L; k < M; k++)
        {
            oup[k] = 0.0;
            i++;
            t1 = L + i;
            tmin = MIN (t1, N);
            for (m = i; m < tmin; m++)
            {
                oup[k] += inp1[m] * inp2[k - m];
            }
        }
    }
    else
    {
        for (k = 0; k < N; k++)
        {
            oup[k] = 0.0;
            for (m = 0; m <= k; m++)
            {
                oup[k] += inp2[m] * inp1[k - m];
            }
        }

        for (k = N; k < M; k++)
        {
            oup[k] = 0.0;
            i++;
            t1 = N + i;
            tmin = MIN (t1, L);
            for (m = i; m < tmin; m++)
            {
                oup[k] += inp2[m] * inp1[k - m];
            }
        }
    }
}


void conv_fft (const conv_object obj, fft_type *inp1, fft_type *inp2, fft_type *oup)
{
    int i, N, L1, L2, ls;
    fft_type *a;
    fft_type *b;
    fft_data *c;
    fft_data *ao;
    fft_data *bo;
    fft_type *co;

    N = obj->clen;
    L1 = obj->ilen1;
    L2 = obj->ilen2;
    ls = L1 + L2 - 1;

    a = (fft_type *)malloc (sizeof (fft_data) * N);
    b = (fft_type *)malloc (sizeof (fft_data) * N);
    c = (fft_data *)malloc (sizeof (fft_data) * N);
    ao = (fft_data *)malloc (sizeof (fft_data) * N);
    bo = (fft_data *)malloc (sizeof (fft_data) * N);
    co = (fft_type *)malloc (sizeof (fft_data) * N);

    for (i = 0; i < N; i++)
    {
        if (i < L1)
        {
            a[i] = inp1[i];
        }
        else
        {
            a[i] = 0.0;
        }

        if (i < L2)
        {
            b[i] = inp2[i];
        }
        else
        {
            b[i] = 0.0;
        }
    }

    fft_r2c_exec (obj->fobj, a, ao);
    fft_r2c_exec (obj->fobj, b, bo);

    for (i = 0; i < N; i++)
    {
        c[i].re = ao[i].re * bo[i].re - ao[i].im * bo[i].im;
        c[i].im = ao[i].im * bo[i].re + ao[i].re * bo[i].im;
    }

    fft_c2r_exec (obj->iobj, c, co);

    for (i = 0; i < ls; i++)
    {
        oup[i] = co[i] / N;
    }

    free (a);
    free (b);
    free (c);
    free (ao);
    free (bo);
    free (co);
}


void free_conv (conv_object object)
{
    free_real_fft (object->fobj);
    free_real_fft (object->iobj);
    free (object);
}
