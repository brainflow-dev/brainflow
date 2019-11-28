#ifndef WAVELIB_H_
#define WAVELIB_H_

#include <stdexcept>

#if defined(_MSC_VER)
#pragma warning(disable : 4200)
#pragma warning(disable : 4996)
#endif

#ifndef fft_type
#define fft_type double
#endif

#ifndef cplx_type
#define cplx_type double
#endif

typedef struct cplx_t
{
    cplx_type re;
    cplx_type im;
} cplx_data;

typedef struct wave_set *wave_object;

wave_object wave_init (const char *wname);

struct wave_set
{
    char wname[50];
    int filtlength; // When all filters are of the same length. [Matlab uses zero-padding to
                    // make all filters of the same length]
    int lpd_len;    // Default filtlength = lpd_len = lpr_len = hpd_len = hpr_len
    int hpd_len;
    int lpr_len;
    int hpr_len;
    double *lpd;
    double *hpd;
    double *lpr;
    double *hpr;
    double params[0];
};

typedef struct fft_t
{
    fft_type re;
    fft_type im;
} fft_data;

typedef struct fft_set *fft_object;

fft_object fft_init (int N, int sgn);

struct fft_set
{
    int N;
    int sgn;
    int factors[64];
    int lf;
    int lt;
    fft_data twiddle[1];
};

typedef struct fft_real_set *fft_real_object;

fft_real_object fft_real_init (int N, int sgn);

struct fft_real_set
{
    fft_object cobj;
    fft_data twiddle2[1];
};

typedef struct conv_set *conv_object;

conv_object conv_init (int N, int L);

struct conv_set
{
    fft_real_object fobj;
    fft_real_object iobj;
    int ilen1;
    int ilen2;
    int clen;
};

typedef struct wt_set *wt_object;

wt_object wt_init (wave_object wave, const char *method, int siglength, int J);

struct wt_set
{
    wave_object wave;
    conv_object cobj;
    char method[10];
    int siglength;      // Length of the original signal.
    int modwtsiglength; // Modified signal length for MODWT
    int outlength;      // Length of the output DWT vector
    int lenlength;      // Length of the Output Dimension Vector "length"
    int J;              // Number of decomposition Levels
    int MaxIter;        // Maximum Iterations J <= MaxIter
    int even;           // even = 1 if signal is of even length. even = 0 otherwise
    char ext[10];       // Type of Extension used - "per" or "sym"
    char cmethod[10];   // Convolution Method - "direct" or "FFT"

    int N; //
    int cfftset;
    int zpad;
    int length[102];
    double *output;
    double params[0];
};

typedef struct wtree_set *wtree_object;

wtree_object wtree_init (wave_object wave, int siglength, int J);

struct wtree_set
{
    wave_object wave;
    conv_object cobj;
    char method[10];
    int siglength; // Length of the original signal.
    int outlength; // Length of the output DWT vector
    int lenlength; // Length of the Output Dimension Vector "length"
    int J;         // Number of decomposition Levels
    int MaxIter;   // Maximum Iterations J <= MaxIter
    int even;      // even = 1 if signal is of even length. even = 0 otherwise
    char ext[10];  // Type of Extension used - "per" or "sym"

    int N; //
    int nodes;
    int cfftset;
    int zpad;
    int length[102];
    double *output;
    int *nodelength;
    int *coeflength;
    double params[0];
};

typedef struct wpt_set *wpt_object;

wpt_object wpt_init (wave_object wave, int siglength, int J);

struct wpt_set
{
    wave_object wave;
    conv_object cobj;
    int siglength; // Length of the original signal.
    int outlength; // Length of the output DWT vector
    int lenlength; // Length of the Output Dimension Vector "length"
    int J;         // Number of decomposition Levels
    int MaxIter;   // Maximum Iterations J <= MaxIter
    int even;      // even = 1 if signal is of even length. even = 0 otherwise
    char ext[10];  // Type of Extension used - "per" or "sym"
    char entropy[20];
    double eparam;

    int N; //
    int nodes;
    int length[102];
    double *output;
    double *costvalues;
    double *basisvector;
    int *nodeindex;
    int *numnodeslevel;
    int *coeflength;
    double params[0];
};


typedef struct cwt_set *cwt_object;

cwt_object cwt_init (const char *wave, double param, int siglength, double dt, int J);

struct cwt_set
{
    char wave[10]; // Wavelet - morl/morlet,paul,dog/dgauss
    int siglength; // Length of Input Data
    int J;         // Total Number of Scales
    double s0; // Smallest scale. It depends on the sampling rate. s0 <= 2 * dt for most wavelets
    double dt; // Sampling Rate
    double dj; // Separation between scales. eg., scale = s0 * 2 ^ ( [0:N-1] *dj ) or scale = s0
               // *[0:N-1] * dj
    char type[10]; // Scale Type - Power or Linear
    int pow;       // Base of Power in case type = pow. Typical value is pow = 2
    int sflag;
    int pflag;
    int npad;
    int mother;
    double m;     // Wavelet parameter param
    double smean; // Input Signal mean

    cplx_data *output;
    double *scale;
    double *period;
    double *coi;
    double params[0];
};

typedef struct wt2_set *wt2_object;

wt2_object wt2_init (wave_object wave, const char *method, int rows, int cols, int J);

struct wt2_set
{
    wave_object wave;
    char method[10];
    int rows;      // Matrix Number of rows
    int cols;      // Matrix Number of columns
    int outlength; // Length of the output DWT vector
    int J;         // Number of decomposition Levels
    int MaxIter;   // Maximum Iterations J <= MaxIter
    char ext[10];  // Type of Extension used - "per" or "sym"
    int coeffaccesslength;

    int N; //
    int *dimensions;
    int *coeffaccess;
    int params[0];
};

void dwt (wt_object wt, const double *inp);

void idwt (wt_object wt, double *dwtop);

double *getDWTmra (wt_object wt, double *wavecoeffs);

void wtree (wtree_object wt, const double *inp);

void dwpt (wpt_object wt, const double *inp);

void idwpt (wpt_object wt, double *dwtop);

void swt (wt_object wt, const double *inp);

void iswt (wt_object wt, double *swtop);

double *getSWTmra (wt_object wt, double *wavecoeffs);

void modwt (wt_object wt, const double *inp);

void imodwt (wt_object wt, double *dwtop);

double *getMODWTmra (wt_object wt, double *wavecoeffs);

void setDWTExtension (wt_object wt, const char *extension);

void setWTREEExtension (wtree_object wt, const char *extension);

void setDWPTExtension (wpt_object wt, const char *extension);

void setDWT2Extension (wt2_object wt, const char *extension);

void setDWPTEntropy (wpt_object wt, const char *entropy, double eparam);

void setWTConv (wt_object wt, const char *cmethod);

int getWTREENodelength (wtree_object wt, int X);

void getWTREECoeffs (wtree_object wt, int X, int Y, double *coeffs, int N);

int getDWPTNodelength (wpt_object wt, int X);

void getDWPTCoeffs (wpt_object wt, int X, int Y, double *coeffs, int N);

void setCWTScales (cwt_object wt, double s0, double dj, const char *type, int power);

void setCWTScaleVector (cwt_object wt, const double *scale, int J, double s0, double dj);

void setCWTPadding (cwt_object wt, int pad);

void cwt (cwt_object wt, const double *inp);

void icwt (cwt_object wt, double *cwtop);

int getCWTScaleLength (int N);

double *dwt2 (wt2_object wt, double *inp);

void idwt2 (wt2_object wt, double *wavecoeff, double *oup);

double *swt2 (wt2_object wt, double *inp);

void iswt2 (wt2_object wt, double *wavecoeffs, double *oup);

double *modwt2 (wt2_object wt, double *inp);

void imodwt2 (wt2_object wt, double *wavecoeff, double *oup);

double *getWT2Coeffs (wt2_object wt, double *wcoeffs, int level, char *type, int *rows, int *cols);

void dispWT2Coeffs (double *A, int row, int col);

void wave_summary (wave_object obj);

void wt_summary (wt_object wt);

void wtree_summary (wtree_object wt);

void wpt_summary (wpt_object wt);

void cwt_summary (cwt_object wt);

void wt2_summary (wt2_object wt);

void wave_free (wave_object object);

void wt_free (wt_object object);

void wtree_free (wtree_object object);

void wpt_free (wpt_object object);

void cwt_free (cwt_object object);

void wt2_free (wt2_object wt);


#endif /* WAVELIB_H_ */
