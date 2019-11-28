#ifndef CWT_H_
#define CWT_H_

#include "wavefunc.h"

void cwavelet (const double *y, int N, double dt, int mother, double param, double s0, double dj,
    int jtot, int npad, double *wave, double *scale, double *period, double *coi);

void psi0 (int mother, double param, double *val, int *real);

double factorial (int N);

double cdelta (int mother, double param, double psi0);

void icwavelet (double *wave, int N, double *scale, int jtot, double dt, double dj, double cdelta,
    double psi0, double *oup);


#endif /* WAVELIB_H_ */
