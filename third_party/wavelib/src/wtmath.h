/*
Copyright (c) 2014, Rafat Hussain
*/
#ifndef WTMATH_H_
#define WTMATH_H_

#include "wavefilt.h"


void dwt_per_stride (double *inp, int N, double *lpd, double *hpd, int lpd_len, double *cA,
    int len_cA, double *cD, int istride, int ostride);

void dwt_sym_stride (double *inp, int N, double *lpd, double *hpd, int lpd_len, double *cA,
    int len_cA, double *cD, int istride, int ostride);

void modwt_per_stride (int M, double *inp, int N, double *filt, int lpd_len, double *cA, int len_cA,
    double *cD, int istride, int ostride);

void swt_per_stride (int M, double *inp, int N, double *lpd, double *hpd, int lpd_len, double *cA,
    int len_cA, double *cD, int istride, int ostride);

void idwt_per_stride (double *cA, int len_cA, double *cD, double *lpr, double *hpr, int lpr_len,
    double *X, int istride, int ostride);

void idwt_sym_stride (double *cA, int len_cA, double *cD, double *lpr, double *hpr, int lpr_len,
    double *X, int istride, int ostride);

void imodwt_per_stride (int M, double *cA, int len_cA, double *cD, double *filt, int lf, double *X,
    int istride, int ostride);

void idwt2_shift (int shift, int rows, int cols, double *lpr, double *hpr, int lf, double *A,
    double *H, double *V, double *D, double *oup);

int upsamp (double *x, int lenx, int M, double *y);

int upsamp2 (double *x, int lenx, int M, double *y);

int downsamp (double *x, int lenx, int M, double *y);

int per_ext (double *sig, int len, int a, double *oup);

int symm_ext (double *sig, int len, int a, double *oup);

void circshift (double *array, int N, int L);

int testSWTlength (int N, int J);

int wmaxiter (int sig_len, int filt_len);

double costfunc (double *x, int N, char *entropy, double p);


#endif /* WAVELIB_H_ */
