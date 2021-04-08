/*
  Copyright (c) 2015, Rafat Hussain
*/
/*
This code is a C translation ( with some modifications) of Wavelet Software provided by
C. Torrence and G. Compo, and is available at URL: http://atoc.colorado.edu/research/wavelets/''.
*/

#include "cwt.h"

double factorial (int N)
{
    static const double fact[41] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800,
        39916800, 479001600, 6227020800, 87178291200, 1307674368000, 20922789888000,
        355687428096000, 6402373705728000, 121645100408832000, 2432902008176640000,
        51090942171709440000.0, 1124000727777607680000.0, 25852016738884976640000.0,
        620448401733239439360000.0, 15511210043330985984000000.0, 403291461126605635584000000.0,
        10888869450418352160768000000.0, 304888344611713860501504000000.0,
        8841761993739701954543616000000.0, 265252859812191058636308480000000.0,
        8222838654177922817725562880000000.0, 263130836933693530167218012160000000.0,
        8683317618811886495518194401280000000.0, 295232799039604140847618609643520000000.0,
        10333147966386144929666651337523200000000.0, 371993326789901217467999448150835200000000.0,
        13763753091226345046315979581580902400000000.0,
        523022617466601111760007224100074291200000000.0,
        20397882081197443358640281739902897356800000000.0,
        815915283247897734345611269596115894272000000000.0};

    if (N > 40 || N < 0)
    {
        printf ("This program is only valid for 0 <= N <= 40 \n");
        return -1.0;
    }

    return fact[N];
}
static void wave_function (int nk, double dt, int mother, double param, double scale1,
    double *kwave, double pi, double *period1, double *coi1, fft_data *daughter)
{

    double norm, expnt, fourier_factor;
    int k, m;
    double temp;
    int sign, re;


    if (mother == 0)
    {
        // MORLET
        if (param < 0.0)
        {
            param = 6.0;
        }
        norm = sqrt (2.0 * pi * scale1 / dt) * pow (pi, -0.25);

        for (k = 1; k <= nk / 2 + 1; ++k)
        {
            temp = (scale1 * kwave[k - 1] - param);
            expnt = -0.5 * temp * temp;
            daughter[k - 1].re = norm * exp (expnt);
            daughter[k - 1].im = 0.0;
        }
        for (k = nk / 2 + 2; k <= nk; ++k)
        {
            daughter[k - 1].re = daughter[k - 1].im = 0.0;
        }
        fourier_factor = (4.0 * pi) / (param + sqrt (2.0 + param * param));
        *period1 = scale1 * fourier_factor;
        *coi1 = fourier_factor / sqrt (2.0);
    }
    else if (mother == 1)
    {
        // PAUL
        if (param < 0.0)
        {
            param = 4.0;
        }
        m = (int)param;
        norm = sqrt (2.0 * pi * scale1 / dt) *
            (pow (2.0, (double)m) / sqrt ((double)(m * factorial (2 * m - 1))));
        for (k = 1; k <= nk / 2 + 1; ++k)
        {
            temp = scale1 * kwave[k - 1];
            expnt = -temp;
            daughter[k - 1].re = norm * pow (temp, (double)m) * exp (expnt);
            daughter[k - 1].im = 0.0;
        }
        for (k = nk / 2 + 2; k <= nk; ++k)
        {
            daughter[k - 1].re = daughter[k - 1].im = 0.0;
        }
        fourier_factor = (4.0 * pi) / (2.0 * m + 1.0);
        *period1 = scale1 * fourier_factor;
        *coi1 = fourier_factor * sqrt (2.0);
    }
    else if (mother == 2)
    {
        if (param < 0.0)
        {
            param = 2.0;
        }
        m = (int)param;

        if (m % 2 == 0)
        {
            re = 1;
        }
        else
        {
            re = 0;
        }

        if (m % 4 == 0 || m % 4 == 1)
        {
            sign = -1;
        }
        else
        {
            sign = 1;
        }


        norm = sqrt (2.0 * pi * scale1 / dt) * sqrt (1.0 / cwt_gamma (m + 0.50));
        norm *= sign;

        if (re == 1)
        {
            for (k = 1; k <= nk; ++k)
            {
                temp = scale1 * kwave[k - 1];
                daughter[k - 1].re = norm * pow (temp, (double)m) * exp (-0.50 * pow (temp, 2.0));
                daughter[k - 1].im = 0.0;
            }
        }
        else if (re == 0)
        {
            for (k = 1; k <= nk; ++k)
            {
                temp = scale1 * kwave[k - 1];
                daughter[k - 1].re = 0.0;
                daughter[k - 1].im = norm * pow (temp, (double)m) * exp (-0.50 * pow (temp, 2.0));
            }
        }
        fourier_factor = (2.0 * pi) * sqrt (2.0 / (2.0 * m + 1.0));
        *period1 = scale1 * fourier_factor;
        *coi1 = fourier_factor / sqrt (2.0);
    }
}

void cwavelet (const double *y, int N, double dt, int mother, double param, double s0, double dj,
    int jtot, int npad, double *wave, double *scale, double *period, double *coi)
{

    int i, j, k, iter;
    double ymean, freq1, pi, period1, coi1;
    double tmp1, tmp2;
    double scale1;
    double *kwave;
    fft_object obj, iobj;
    fft_data *ypad, *yfft, *daughter;

    (void)s0;
    (void)dj; /* yes, we need these parameters unused */

    pi = 4.0 * atan (1.0);

    if (npad < N)
    {
        throw std::runtime_error ("npad must be >= N");
    }

    obj = fft_init (npad, 1);
    iobj = fft_init (npad, -1);

    ypad = (fft_data *)malloc (sizeof (fft_data) * npad);
    yfft = (fft_data *)malloc (sizeof (fft_data) * npad);
    daughter = (fft_data *)malloc (sizeof (fft_data) * npad);
    kwave = (double *)malloc (sizeof (double) * npad);

    ymean = 0.0;

    for (i = 0; i < N; ++i)
    {
        ymean += y[i];
    }

    ymean /= N;

    for (i = 0; i < N; ++i)
    {
        ypad[i].re = y[i] - ymean;
        ypad[i].im = 0.0;
    }

    for (i = N; i < npad; ++i)
    {
        ypad[i].re = ypad[i].im = 0.0;
    }


    // Find FFT of the input y (ypad)

    fft_exec (obj, ypad, yfft);

    for (i = 0; i < npad; ++i)
    {
        yfft[i].re /= (double)npad;
        yfft[i].im /= (double)npad;
    }


    // Construct the wavenumber array

    freq1 = 2.0 * pi / ((double)npad * dt);
    kwave[0] = 0.0;

    for (i = 1; i < npad / 2 + 1; ++i)
    {
        kwave[i] = i * freq1;
    }

    for (i = npad / 2 + 1; i < npad; ++i)
    {
        kwave[i] = -kwave[npad - i];
    }


    // Main loop

    for (j = 1; j <= jtot; ++j)
    {
        scale1 = scale[j - 1]; // = s0*pow(2.0, (double)(j - 1)*dj);
        wave_function (npad, dt, mother, param, scale1, kwave, pi, &period1, &coi1, daughter);
        period[j - 1] = period1;
        for (k = 0; k < npad; ++k)
        {
            tmp1 = daughter[k].re * yfft[k].re - daughter[k].im * yfft[k].im;
            tmp2 = daughter[k].re * yfft[k].im + daughter[k].im * yfft[k].re;
            daughter[k].re = tmp1;
            daughter[k].im = tmp2;
        }
        fft_exec (iobj, daughter, ypad);
        iter = 2 * (j - 1) * N;
        for (i = 0; i < N; ++i)
        {
            wave[iter + 2 * i] = ypad[i].re;
            wave[iter + 2 * i + 1] = ypad[i].im;
        }
    }


    for (i = 1; i <= (N + 1) / 2; ++i)
    {
        coi[i - 1] = coi1 * dt * ((double)i - 1.0);
        coi[N - i] = coi[i - 1];
    }


    free (kwave);
    free (ypad);
    free (yfft);
    free (daughter);

    free_fft (obj);
    free_fft (iobj);
}

void psi0 (int mother, double param, double *val, int *real)
{
    double pi, coeff;
    int m, sign;

    m = (int)param;
    pi = 4.0 * atan (1.0);

    if (mother == 0)
    {
        // Morlet
        *val = 1.0 / sqrt (sqrt (pi));
        *real = 1;
    }
    else if (mother == 1)
    {
        // Paul
        if (m % 2 == 0)
        {
            *real = 1;
        }
        else
        {
            *real = 0;
        }

        if (m % 4 == 0 || m % 4 == 1)
        {
            sign = 1;
        }
        else
        {
            sign = -1;
        }
        *val = sign * pow (2.0, (double)m) * factorial (m) / (sqrt (pi * factorial (2 * m)));
    }
    else if (mother == 2)
    {
        // D.O.G
        *real = 1;

        if (m % 2 == 0)
        {
            if (m % 4 == 0)
            {
                sign = -1;
            }
            else
            {
                sign = 1;
            }
            coeff = sign * pow (2.0, (double)m / 2) / cwt_gamma (0.5);
            *val = coeff * cwt_gamma (((double)m + 1.0) / 2.0) / sqrt (cwt_gamma (m + 0.50));
        }
        else
        {
            *val = 0;
        }
    }
}

static int maxabs (double *array, int N)
{
    double maxval, temp;
    int i, index;
    maxval = 0.0;
    index = -1;

    for (i = 0; i < N; ++i)
    {
        temp = fabs (array[i]);
        if (temp >= maxval)
        {
            maxval = temp;
            index = i;
        }
    }

    return index;
}


double cdelta (int mother, double param, double psi0)
{
    int N = 0, i = 0, j = 0, iter = 0;
    double *delta = NULL, *scale = NULL, *period = NULL, *wave = NULL, *coi = NULL, *mval = NULL;
    double den = 0.0, cdel = 0.0;
    double subscale = 0.0, dt = 0.0, dj = 0.0, s0 = 0.0;
    int jtot = 0;
    int maxarr = 0;

    subscale = 8.0;
    dt = 0.25;
    if (mother == 0)
    {
        N = 16;
        s0 = dt / 4;
    }
    else if (mother == 1)
    {
        N = 16;
        s0 = dt / 4.0;
    }
    else if (mother == 2)
    {
        s0 = dt / 8.0;
        N = 256;
        if (param == 2.0)
        {
            subscale = 16.0;
            s0 = dt / 16.0;
            N = 2048;
        }
    }

    dj = 1.0 / subscale;
    jtot = 16 * (int)subscale;

    delta = (double *)malloc (sizeof (double) * N);
    wave = (double *)malloc (sizeof (double) * 2 * N * jtot);
    coi = (double *)malloc (sizeof (double) * N);
    scale = (double *)malloc (sizeof (double) * jtot);
    period = (double *)malloc (sizeof (double) * jtot);
    mval = (double *)malloc (sizeof (double) * N);


    delta[0] = 1;

    for (i = 1; i < N; ++i)
    {
        delta[i] = 0;
    }

    for (i = 0; i < jtot; ++i)
    {
        scale[i] = s0 * pow (2.0, (double)(i)*dj);
    }

    cwavelet (delta, N, dt, mother, param, s0, dj, jtot, N, wave, scale, period, coi);

    for (i = 0; i < N; ++i)
    {
        mval[i] = 0;
    }

    for (j = 0; j < jtot; ++j)
    {
        iter = 2 * j * N;
        den = sqrt (scale[j]);
        for (i = 0; i < N; ++i)
        {
            mval[i] += wave[iter + 2 * i] / den;
        }
    }


    maxarr = maxabs (mval, N);

    cdel = sqrt (dt) * dj * mval[maxarr] / psi0;

    free (delta);
    free (wave);

    free (scale);
    free (period);
    free (coi);
    free (mval);

    return cdel;
}

void icwavelet (double *wave, int N, double *scale, int jtot, double dt, double dj, double cdelta,
    double psi0, double *oup)
{
    int i, j, iter;
    double den, coeff;

    coeff = sqrt (dt) * dj / (cdelta * psi0);

    for (i = 0; i < N; ++i)
    {
        oup[i] = 0.0;
    }

    for (j = 0; j < jtot; ++j)
    {
        iter = 2 * j * N;
        den = sqrt (scale[j]);
        for (i = 0; i < N; ++i)
        {
            oup[i] += wave[iter + 2 * i] / den;
        }
    }

    for (i = 0; i < N; ++i)
    {
        oup[i] *= coeff;
    }
}
