/*****************************************************************************

        TestAccuracy.h
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (ffft_test_TestAccuracy_HEADER_INCLUDED)
#define	ffft_test_TestAccuracy_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace ffft
{
namespace test
{



template <class FO>
class TestAccuracy
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef	typename FO::DataType	DataType;
	typedef	long double	BigFloat;	// To get maximum accuracy during intermediate calculations

   static int		perform_test_single_object (FO &fft);
   static int		perform_test_d (FO &fft, const char *class_name_0);
   static int		perform_test_i (FO &fft, const char *class_name_0);
   static int		perform_test_di (FO &fft, const char *class_name_0);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	enum {			NBR_ACC_TESTS	= 10 * 1000 * 1000	};
   enum {         MAX_NBR_TESTS  = 10000  };

   static void		compute_tf (DataType s [], const DataType x [], long length);
	static void		compute_itf (DataType x [], const DataType s [], long length);
	static int		compare_vect_display (const DataType x_ptr [], const DataType y_ptr [], long len, BigFloat &max_err_rel);
	static BigFloat
						compute_power (const DataType x_ptr [], long len);
	static BigFloat
						compute_power (const DataType x_ptr [], const DataType y_ptr [], long len);
	static void		compare_vect (const DataType x_ptr [], const DataType y_ptr [], BigFloat &power, long &max_err_pos, long len);



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						TestAccuracy ();
						TestAccuracy (const TestAccuracy &other);
	TestAccuracy &	operator = (const TestAccuracy &other);
	bool				operator == (const TestAccuracy &other);
	bool				operator != (const TestAccuracy &other);

};	// class TestAccuracy



}	// namespace test
}	// namespace ffft



#include	"ffft/test/TestAccuracy.hpp"



#endif	// ffft_test_TestAccuracy_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
