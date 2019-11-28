#include "wavefunc.h"

void meyer (int N, double lb, double ub, double *phi, double *psi, double *tgrid)
{
    int M, i;
    double *w;
    double delta, j;
    double theta, x, x2, x3, x4, v, cs, sn;
    double wf;
    fft_data *phiw, *psiw, *oup;
    fft_object obj;

    M = divideby (N, 2);

    if (M == 0)
    {
        throw std::runtime_error ("size of wavelet must be power of 2");
    }
    if (lb >= ub)
    {
        throw std::runtime_error ("upper bound must be greater than lower bound");
    }

    obj = fft_init (N, -1);
    w = (double *)malloc (sizeof (double) * N);
    phiw = (fft_data *)malloc (sizeof (fft_data) * N);
    psiw = (fft_data *)malloc (sizeof (fft_data) * N);
    oup = (fft_data *)malloc (sizeof (fft_data) * N);

    delta = 2 * (ub - lb) / PI2;

    j = (double)N;
    j *= -1.0;

    for (i = 0; i < N; ++i)
    {
        w[i] = j / delta;
        j += 2.0;
        psiw[i].re = psiw[i].im = 0.0;
        phiw[i].re = phiw[i].im = 0.0;
    }


    for (i = 0; i < N; ++i)
    {
        wf = fabs (w[i]);
        if (wf <= PI2 / 3.0)
        {
            phiw[i].re = 1.0;
        }
        if (wf > PI2 / 3.0 && wf <= 2 * PI2 / 3.0)
        {
            x = (3 * wf / PI2) - 1.0;
            x2 = x * x;
            x3 = x2 * x;
            x4 = x3 * x;
            v = x4 * (35 - 84 * x + 70 * x2 - 20 * x3);
            theta = v * PI2 / 4.0;
            cs = cos (theta);
            sn = sin (theta);

            phiw[i].re = cs;
            psiw[i].re = cos (w[i] / 2.0) * sn;
            psiw[i].im = sin (w[i] / 2.0) * sn;
        }
        if (wf > 2.0 * PI2 / 3.0 && wf <= 4 * PI2 / 3.0)
        {
            x = (1.5 * wf / PI2) - 1.0;
            x2 = x * x;
            x3 = x2 * x;
            x4 = x3 * x;
            v = x4 * (35 - 84 * x + 70 * x2 - 20 * x3);
            theta = v * PI2 / 4.0;
            cs = cos (theta);

            psiw[i].re = cos (w[i] / 2.0) * cs;
            psiw[i].im = sin (w[i] / 2.0) * cs;
        }
    }


    nsfft_exec (obj, phiw, oup, lb, ub, tgrid);


    for (i = 0; i < N; ++i)
    {
        phi[i] = oup[i].re / N;
    }

    nsfft_exec (obj, psiw, oup, lb, ub, tgrid);


    for (i = 0; i < N; ++i)
    {
        psi[i] = oup[i].re / N;
    }


    free (oup);
    free (phiw);
    free (psiw);
    free (w);
}

void gauss (int N, int p, double lb, double ub, double *psi, double *t)
{
    double delta, num, den, t2, t4;
    int i;

    if (lb >= ub)
    {
        throw std::runtime_error ("upper bound must be greater than lower bound");
    }

    t[0] = lb;
    t[N - 1] = ub;
    delta = (ub - lb) / (N - 1);
    for (i = 1; i < N - 1; ++i)
    {
        t[i] = lb + delta * i;
    }

    den = sqrt (cwt_gamma (p + 0.5));

    if ((p + 1) % 2 == 0)
    {
        num = 1.0;
    }
    else
    {
        num = -1.0;
    }

    num /= den;

    // printf("\n%g\n",num);

    if (p == 1)
    {
        for (i = 0; i < N; ++i)
        {
            psi[i] = -t[i] * exp (-t[i] * t[i] / 2.0) * num;
        }
    }
    else if (p == 2)
    {
        for (i = 0; i < N; ++i)
        {
            t2 = t[i] * t[i];
            psi[i] = (-1.0 + t2) * exp (-t2 / 2.0) * num;
        }
    }
    else if (p == 3)
    {
        for (i = 0; i < N; ++i)
        {
            t2 = t[i] * t[i];
            psi[i] = t[i] * (3.0 - t2) * exp (-t2 / 2.0) * num;
        }
    }
    else if (p == 4)
    {
        for (i = 0; i < N; ++i)
        {
            t2 = t[i] * t[i];
            psi[i] = (t2 * t2 - 6.0 * t2 + 3.0) * exp (-t2 / 2.0) * num;
        }
    }
    else if (p == 5)
    {
        for (i = 0; i < N; ++i)
        {
            t2 = t[i] * t[i];
            psi[i] = t[i] * (-t2 * t2 + 10.0 * t2 - 15.0) * exp (-t2 / 2.0) * num;
        }
    }
    else if (p == 6)
    {
        for (i = 0; i < N; ++i)
        {
            t2 = t[i] * t[i];
            psi[i] = (t2 * t2 * t2 - 15.0 * t2 * t2 + 45.0 * t2 - 15.0) * exp (-t2 / 2.0) * num;
        }
    }
    else if (p == 7)
    {
        for (i = 0; i < N; ++i)
        {
            t2 = t[i] * t[i];
            psi[i] = t[i] * (-t2 * t2 * t2 + 21.0 * t2 * t2 - 105.0 * t2 + 105.0) *
                exp (-t2 / 2.0) * num;
        }
    }
    else if (p == 8)
    {
        for (i = 0; i < N; ++i)
        {
            t2 = t[i] * t[i];
            t4 = t2 * t2;
            psi[i] = (t4 * t4 - 28.0 * t4 * t2 + 210.0 * t4 - 420.0 * t2 + 105.0) *
                exp (-t2 / 2.0) * num;
        }
    }
    else if (p == 9)
    {
        for (i = 0; i < N; ++i)
        {
            t2 = t[i] * t[i];
            t4 = t2 * t2;
            psi[i] = t[i] * (-t4 * t4 + 36.0 * t4 * t2 - 378.0 * t4 + 1260.0 * t2 - 945.0) *
                exp (-t2 / 2.0) * num;
        }
    }
    else if (p == 10)
    {
        for (i = 0; i < N; ++i)
        {
            t2 = t[i] * t[i];
            t4 = t2 * t2;
            psi[i] = (t4 * t4 * t2 - 45.0 * t4 * t4 + 630.0 * t4 * t2 - 3150.0 * t4 + 4725.0 * t2 -
                         945.0) *
                exp (-t2 / 2.0) * num;
        }
    }
    else
    {
        throw std::runtime_error (
            "gaussian derivative wavelet is only available for derivatives 1 to 10");
    }
}

void mexhat (int N, double lb, double ub, double *psi, double *t)
{
    gauss (N, 2, lb, ub, psi, t);
}

void morlet (int N, double lb, double ub, double *psi, double *t)
{
    int i;
    double delta;

    if (lb >= ub)
    {
        throw std::runtime_error ("upper bound must be greater than lower bound");
    }

    t[0] = lb;
    t[N - 1] = ub;
    delta = (ub - lb) / (N - 1);
    for (i = 1; i < N - 1; ++i)
    {
        t[i] = lb + delta * i;
    }

    for (i = 0; i < N; ++i)
    {
        psi[i] = exp (-t[i] * t[i] / 2.0) * cos (5 * t[i]);
    }
}
