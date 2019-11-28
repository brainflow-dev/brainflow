/*
 * hsfft.h
 *
 *  Created on: Apr 14, 2013
 *      Author: Rafat Hussain
 */

#ifndef HSFFT_H_
#define HSFFT_H_

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../header/wavelib.h"

#define PI2 6.28318530717958647692528676655900577

#ifndef fft_type
#define fft_type double
#endif

/*
#define SADD(a,b) ((a)+(b))

#define SSUB(a,b) ((a)+(b))

#define SMUL(a,b) ((a)*(b))
*/

fft_object fft_init (int N, int sgn);

void fft_exec (fft_object obj, fft_data *inp, fft_data *oup);

int divideby (int M, int d);

int dividebyN (int N);

// void arrrev(int M, int* arr);

int factors (int M, int *arr);

void twiddle (fft_data *sig, int N, int radix);

void longvectorN (fft_data *sig, int *array, int M);

void free_fft (fft_object object);


#endif /* HSFFT_H_ */
