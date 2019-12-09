/*****************************************************************************

        TestHelperNormal.h
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (ffft_test_TestHelperNormal_HEADER_INCLUDED)
#define	ffft_test_TestHelperNormal_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"ffft/FFTReal.h"



namespace ffft
{
namespace test
{



template <class DT>
class TestHelperNormal
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef	DT	DataType;
	typedef	FFTReal <DataType>	FftType;

   static void    perform_test_accuracy (int &ret_val);
   static void    perform_test_speed (int &ret_val);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						TestHelperNormal ();
						TestHelperNormal (const TestHelperNormal &other);
	TestHelperNormal &
						operator = (const TestHelperNormal &other);
	bool				operator == (const TestHelperNormal &other);
	bool				operator != (const TestHelperNormal &other);

};	// class TestHelperNormal



}	// namespace test
}	// namespace ffft



#include	"ffft/test/TestHelperNormal.hpp"



#endif	// ffft_test_TestHelperNormal_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
