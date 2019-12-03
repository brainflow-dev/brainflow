/*****************************************************************************

        TestAccuracy.hpp
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (ffft_test_TestAccuracy_CURRENT_CODEHEADER)
	#error Recursive inclusion of TestAccuracy code header.
#endif
#define	ffft_test_TestAccuracy_CURRENT_CODEHEADER

#if ! defined (ffft_test_TestAccuracy_CODEHEADER_INCLUDED)
#define	ffft_test_TestAccuracy_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"ffft/def.h"
#include	"ffft/test/fnc.h"
#include	"ffft/test/TestWhiteNoiseGen.h"

#include	<typeinfo>
#include	<vector>

#include	<cmath>
#include	<cstdio>



namespace ffft
{
namespace test
{



static const double	TestAccuracy_LN10	= 2.3025850929940456840179914546844;



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <class FO>
int	TestAccuracy <FO>::perform_test_single_object (FO &fft)
{
	assert (&fft != 0);

	using namespace std;

	int				ret_val = 0;

	const std::type_info &	ti = typeid (fft);
	const char *	class_name_0 = ti.name ();

	if (ret_val == 0)
	{
		ret_val = perform_test_d (fft, class_name_0);
	}
	if (ret_val == 0)
	{
		ret_val = perform_test_i (fft, class_name_0);
	}
	if (ret_val == 0)
	{
		ret_val = perform_test_di (fft, class_name_0);
	}

	if (ret_val == 0)
	{
		printf ("\n");
	}

	return (ret_val);
}



template <class FO>
int	TestAccuracy <FO>::perform_test_d (FO &fft, const char *class_name_0)
{
	assert (&fft != 0);
   assert (class_name_0 != 0);

	using namespace std;

	int				ret_val = 0;
	const long		len = fft.get_length ();
   const long     nbr_tests = limit (
      NBR_ACC_TESTS / len / len,
      1L,
      static_cast <long> (MAX_NBR_TESTS)
   );

	printf ("Testing %s::do_fft () [%ld samples]... ", class_name_0, len);
	fflush (stdout);
	TestWhiteNoiseGen <DataType>	noise;
	std::vector <DataType>	x (len);
	std::vector <DataType>	s1 (len);
	std::vector <DataType>	s2 (len);
	BigFloat			err_avg = 0;

	for (long test = 0; test < nbr_tests && ret_val == 0; ++ test)
	{
		noise.generate (&x [0], len);
		fft.do_fft (&s1 [0], &x [0]);
		compute_tf (&s2 [0], &x [0], len);

		BigFloat			max_err;
		compare_vect_display (&s1 [0], &s2 [0], len, max_err);
		err_avg += max_err;
	}
	err_avg /= NBR_ACC_TESTS;

	printf ("done.\n");
	printf (
		"Average maximum error: %.6f %% (%f dB)\n",
		static_cast <double> (err_avg * 100),
		static_cast <double> ((20 / TestAccuracy_LN10) * log (err_avg + 1e-300))
	);

	return (ret_val);
}



template <class FO>
int	TestAccuracy <FO>::perform_test_i (FO &fft, const char *class_name_0)
{
	assert (&fft != 0);
   assert (class_name_0 != 0);

	using namespace std;

	int				ret_val = 0;
	const long		len = fft.get_length ();
   const long     nbr_tests = limit (
      NBR_ACC_TESTS / len / len,
      10L,
      static_cast <long> (MAX_NBR_TESTS)
   );

	printf ("Testing %s::do_ifft () [%ld samples]... ", class_name_0, len);
	fflush (stdout);
	TestWhiteNoiseGen <DataType>	noise;
	std::vector <DataType>	s (len);
	std::vector <DataType>	x1 (len);
	std::vector <DataType>	x2 (len);
	BigFloat			err_avg = 0;

	for (long test = 0; test < nbr_tests && ret_val == 0; ++ test)
	{
		noise.generate (&s [0], len);
		fft.do_ifft (&s [0], &x1 [0]);
		compute_itf (&x2 [0], &s [0], len);

		BigFloat			max_err;
		compare_vect_display (&x1 [0], &x2 [0], len, max_err);
		err_avg += max_err;
	}
	err_avg /= NBR_ACC_TESTS;

	printf ("done.\n");
	printf (
		"Average maximum error: %.6f %% (%f dB)\n",
		static_cast <double> (err_avg * 100),
		static_cast <double> ((20 / TestAccuracy_LN10) * log (err_avg + 1e-300))
	);

	return (ret_val);
}



template <class FO>
int	TestAccuracy <FO>::perform_test_di (FO &fft, const char *class_name_0)
{
	assert (&fft != 0);
   assert (class_name_0 != 0);

	using namespace std;

	int				ret_val = 0;
	const long		len = fft.get_length ();
   const long     nbr_tests = limit (
      NBR_ACC_TESTS / len / len,
      1L,
      static_cast <long> (MAX_NBR_TESTS)
   );

	printf (
		"Testing %s::do_fft () / do_ifft () / rescale () [%ld samples]... ",
		class_name_0,
		len
	);
	fflush (stdout);
	TestWhiteNoiseGen <DataType>	noise;
	std::vector <DataType>	x (len);
	std::vector <DataType>	s (len);
	std::vector <DataType>	y (len);
	BigFloat			err_avg = 0;

	for (long test = 0; test < nbr_tests && ret_val == 0; ++ test)
	{
		noise.generate (&x [0], len);
		fft.do_fft (&s [0], &x [0]);
		fft.do_ifft (&s [0], &y [0]);
		fft.rescale (&y [0]);

		BigFloat			max_err;
		compare_vect_display (&x [0], &y [0], len, max_err);
		err_avg += max_err;
	}
	err_avg /= NBR_ACC_TESTS;

	printf ("done.\n");
	printf (
		"Average maximum error: %.6f %% (%f dB)\n",
		static_cast <double> (err_avg * 100),
		static_cast <double> ((20 / TestAccuracy_LN10) * log (err_avg + 1e-300))
	);

	return (ret_val);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



// Positive transform
template <class FO>
void	TestAccuracy <FO>::compute_tf (DataType s [], const DataType x [], long length)
{
	assert (s != 0);
	assert (x != 0);
	assert (length >= 2);
	assert ((length & 1) == 0);

	const long		nbr_bins = length >> 1;

	// DC and Nyquist
	BigFloat			dc = 0;
	BigFloat			ny = 0;
	for (long pos = 0; pos < length; pos += 2)
	{
		const BigFloat	even = x [pos    ];
		const BigFloat	odd  = x [pos + 1];
		dc += even + odd;
		ny += even - odd;
	}
	s [0       ] = static_cast <DataType> (dc);
	s [nbr_bins] = static_cast <DataType> (ny);

	// Regular bins
	for (long bin = 1; bin < nbr_bins; ++ bin)
	{
		BigFloat			sum_r = 0;
		BigFloat			sum_i = 0;

		const BigFloat	m = bin * static_cast <BigFloat> (2 * PI) / length;

		for (long pos = 0; pos < length; ++pos)
		{
			using namespace std;

			const BigFloat	phase = pos * m;
			const BigFloat	e_r = cos (phase);
			const BigFloat	e_i = sin (phase);

			sum_r += x [pos] * e_r;
			sum_i += x [pos] * e_i;
		}

		s [           bin] = static_cast <DataType> (sum_r);
		s [nbr_bins + bin] = static_cast <DataType> (sum_i);
	}
}



// Negative transform
template <class FO>
void	TestAccuracy <FO>::compute_itf (DataType x [], const DataType s [], long length)
{
	assert (s != 0);
	assert (x != 0);
	assert (length >= 2);
	assert ((length & 1) == 0);

	const long		nbr_bins = length >> 1;

	// DC and Nyquist
	BigFloat			dc = s [0       ];
	BigFloat			ny = s [nbr_bins];

	// Regular bins
	for (long pos = 0; pos < length; ++pos)
	{
		BigFloat				sum = dc + ny * (1 - 2 * (pos & 1));

		const BigFloat		m = pos * static_cast <BigFloat> (-2 * PI) / length;

		for (long bin = 1; bin < nbr_bins; ++ bin)
		{
			using namespace std;

			const BigFloat	phase = bin * m;
			const BigFloat	e_r = cos (phase);
			const BigFloat	e_i = sin (phase);

			sum += 2 * (  e_r * s [bin           ]
			            - e_i * s [bin + nbr_bins]);
		}

		x [pos] = static_cast <DataType> (sum);
	}
}



template <class FO>
int	TestAccuracy <FO>::compare_vect_display (const DataType x_ptr [], const DataType y_ptr [], long len, BigFloat &max_err_rel)
{
	assert (x_ptr != 0);
	assert (y_ptr != 0);
	assert (len > 0);
	assert (&max_err_rel != 0);

	using namespace std;

	int				ret_val = 0;

	BigFloat			power = compute_power (&x_ptr [0], &y_ptr [0], len);
	BigFloat			power_dif;
	long				max_err_pos;
	compare_vect (&x_ptr [0], &y_ptr [0], power_dif, max_err_pos, len);

	if (power == 0)
	{
		power = power_dif;
	}
	const BigFloat	power_err_rel = power_dif / power;

	BigFloat	      max_err = 0;
	max_err_rel = 0;
	if (max_err_pos >= 0)
	{
      max_err = y_ptr [max_err_pos] - x_ptr [max_err_pos];
		max_err_rel = 2 * fabs (max_err) / (  fabs (y_ptr [max_err_pos])
		                                    + fabs (x_ptr [max_err_pos]));
	}

	if (power_err_rel > 0.001)
	{
		printf ("Power error  : %f (%.6f %%)\n",
			static_cast <double> (power_err_rel),
			static_cast <double> (power_err_rel * 100)
		);
		if (max_err_pos >= 0)
		{
			printf (
				"Maximum error: %f - %f = %f (%f)\n",
				static_cast <double> (y_ptr [max_err_pos]),
				static_cast <double> (x_ptr [max_err_pos]),
				static_cast <double> (max_err),
				static_cast <double> (max_err_pos)
			);
		}
	}

	return (ret_val);
}



template <class FO>
typename TestAccuracy <FO>::BigFloat	TestAccuracy <FO>::compute_power (const DataType x_ptr [], long len)
{
	assert (x_ptr != 0);
	assert (len > 0);

	BigFloat		power = 0;
	for (long pos = 0; pos < len; ++pos)
	{
		const BigFloat	val = x_ptr [pos];

		power += val * val;
	}

	using namespace std;

	power = sqrt (power) / len;

	return (power);
}



template <class FO>
typename TestAccuracy <FO>::BigFloat	TestAccuracy <FO>::compute_power (const DataType x_ptr [], const DataType y_ptr [], long len)
{
	assert (x_ptr != 0);
	assert (y_ptr != 0);
	assert (len > 0);

	return ((compute_power (x_ptr, len) + compute_power (y_ptr, len)) * 0.5);
}



template <class FO>
void	TestAccuracy <FO>::compare_vect (const DataType x_ptr [], const DataType y_ptr [], BigFloat &power, long &max_err_pos, long len)
{
	assert (x_ptr != 0);
	assert (y_ptr != 0);
	assert (len > 0);
	assert (&power != 0);
	assert (&max_err_pos != 0);

	power = 0;
	BigFloat			max_dif2 = 0;
	max_err_pos = -1;

	for (long pos = 0; pos < len; ++pos)
	{
		const BigFloat	x = x_ptr [pos];
		const BigFloat	y = y_ptr [pos];
		const BigFloat	dif = y - x;
		const BigFloat	dif2 = dif * dif;

		power += dif2;
		if (dif2 > max_dif2)
		{
			max_err_pos = pos;
			max_dif2 = dif2;
		}
	}

	using namespace std;

	power = sqrt (power) / len;
}



}	// namespace test
}	// namespace ffft



#endif	// ffft_test_TestAccuracy_CODEHEADER_INCLUDED

#undef ffft_test_TestAccuracy_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
