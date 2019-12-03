/*****************************************************************************

        TestSpeed.hpp
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (ffft_test_TestSpeed_CURRENT_CODEHEADER)
	#error Recursive inclusion of TestSpeed code header.
#endif
#define	ffft_test_TestSpeed_CURRENT_CODEHEADER

#if ! defined (ffft_test_TestSpeed_CODEHEADER_INCLUDED)
#define	ffft_test_TestSpeed_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"ffft/test/fnc.h"
#include	"ffft/test/TestWhiteNoiseGen.h"
#include	"stopwatch/StopWatch.h"

#include	<typeinfo>

#include	<cstdio>



namespace ffft
{
namespace test
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <class FO>
int	TestSpeed <FO>::perform_test_single_object (FO &fft)
{
	assert (&fft != 0);

   int            ret_val = 0;

	const std::type_info &	ti = typeid (fft);
	const char *	class_name_0 = ti.name ();

   if (ret_val == 0)
   {
	   perform_test_d (fft, class_name_0);
   }
   if (ret_val == 0)
   {
	   perform_test_i (fft, class_name_0);
   }
	if (ret_val == 0)
   {
      perform_test_di (fft, class_name_0);
   }

   if (ret_val == 0)
   {
      printf ("\n");
   }

   return (ret_val);
}



template <class FO>
int	TestSpeed <FO>::perform_test_d (FO &fft, const char *class_name_0)
{
	assert (&fft != 0);
   assert (class_name_0 != 0);

	const long		len = fft.get_length ();
   const long     nbr_tests = limit (
      static_cast <long> (NBR_SPD_TESTS / len / len),
      1L,
      static_cast <long> (MAX_NBR_TESTS)
   );

	TestWhiteNoiseGen <DataType>	noise;
	std::vector <DataType>	x (len, 0);
	std::vector <DataType>	s (len);
	noise.generate (&x [0], len);

   printf (
		"%s::do_fft () speed test [%ld samples]... ",
		class_name_0,
		len
	);
	fflush (stdout);

	stopwatch::StopWatch	chrono;
	chrono.start ();
	for (long test = 0; test < nbr_tests; ++ test)
	{
		fft.do_fft (&s [0], &x [0]);
		chrono.stop_lap ();
	}

	printf ("%.1f clocks/sample\n", chrono.get_time_best_lap (len));

	return (0);
}



template <class FO>
int	TestSpeed <FO>::perform_test_i (FO &fft, const char *class_name_0)
{
	assert (&fft != 0);
   assert (class_name_0 != 0);

	const long		len = fft.get_length ();
   const long     nbr_tests = limit (
      static_cast <long> (NBR_SPD_TESTS / len / len),
      1L,
      static_cast <long> (MAX_NBR_TESTS)
   );

	TestWhiteNoiseGen <DataType>	noise;
	std::vector <DataType>	x (len);
	std::vector <DataType>	s (len, 0);
	noise.generate (&s [0], len);

   printf (
		"%s::do_ifft () speed test [%ld samples]... ",
		class_name_0,
		len
	);
	fflush (stdout);

	stopwatch::StopWatch	chrono;
	chrono.start ();
	for (long test = 0; test < nbr_tests; ++ test)
	{
		fft.do_ifft (&s [0], &x [0]);
		chrono.stop_lap ();
	}

	printf ("%.1f clocks/sample\n", chrono.get_time_best_lap (len));

	return (0);
}



template <class FO>
int	TestSpeed <FO>::perform_test_di (FO &fft, const char *class_name_0)
{
	assert (&fft != 0);
   assert (class_name_0 != 0);

	const long		len = fft.get_length ();
   const long     nbr_tests = limit (
      static_cast <long> (NBR_SPD_TESTS / len / len),
      1L,
      static_cast <long> (MAX_NBR_TESTS)
   );

	TestWhiteNoiseGen <DataType>	noise;
	std::vector <DataType>	x (len, 0);
	std::vector <DataType>	s (len);
	std::vector <DataType>	y (len);
	noise.generate (&x [0], len);

   printf (
		"%s::do_fft () / do_ifft () / rescale () speed test [%ld samples]... ",
		class_name_0,
		len
	);
	fflush (stdout);

	stopwatch::StopWatch	chrono;

	chrono.start ();
	for (long test = 0; test < nbr_tests; ++ test)
	{
		fft.do_fft (&s [0], &x [0]);
		fft.do_ifft (&s [0], &y [0]);
		fft.rescale (&y [0]);
		chrono.stop_lap ();
	}

	printf ("%.1f clocks/sample\n", chrono.get_time_best_lap (len));

	return (0);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}	// namespace test
}	// namespace ffft



#endif	// ffft_test_TestSpeed_CODEHEADER_INCLUDED

#undef ffft_test_TestSpeed_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
