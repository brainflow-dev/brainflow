/*****************************************************************************

        TestSpeed.h
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (ffft_test_TestSpeed_HEADER_INCLUDED)
#define	ffft_test_TestSpeed_HEADER_INCLUDED

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
class TestSpeed
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef	typename FO::DataType	DataType;

   static int		perform_test_single_object (FO &fft);
   static int		perform_test_d (FO &fft, const char *class_name_0);
   static int		perform_test_i (FO &fft, const char *class_name_0);
   static int		perform_test_di (FO &fft, const char *class_name_0);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	enum {			NBR_SPD_TESTS	= 10 * 1000 * 1000	};
   enum {         MAX_NBR_TESTS  = 10000  };



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						TestSpeed ();
						~TestSpeed ();
						TestSpeed (const TestSpeed &other);
	TestSpeed &		operator = (const TestSpeed &other);
	bool				operator == (const TestSpeed &other);
	bool				operator != (const TestSpeed &other);

};	// class TestSpeed



}	// namespace test
}	// namespace ffft



#include	"ffft/test/TestSpeed.hpp"



#endif	// ffft_test_TestSpeed_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
