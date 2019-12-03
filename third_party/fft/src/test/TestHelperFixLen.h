/*****************************************************************************

        TestHelperFixLen.h
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (ffft_test_TestHelperFixLen_HEADER_INCLUDED)
#define	ffft_test_TestHelperFixLen_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"ffft/FFTRealFixLen.h"



namespace ffft
{
namespace test
{



template <int L>
class TestHelperFixLen
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef  FFTRealFixLen <L> FftType;

   static void    perform_test_accuracy (int &ret_val);
   static void    perform_test_speed (int &ret_val);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						TestHelperFixLen ();
						TestHelperFixLen (const TestHelperFixLen &other);
	TestHelperFixLen &
						operator = (const TestHelperFixLen &other);
	bool				operator == (const TestHelperFixLen &other);
	bool				operator != (const TestHelperFixLen &other);

};	// class TestHelperFixLen



}	// namespace test
}	// namespace ffft



#include	"ffft/test/TestHelperFixLen.hpp"



#endif	// ffft_test_TestHelperFixLen_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
