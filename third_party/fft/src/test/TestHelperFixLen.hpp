/*****************************************************************************

        TestHelperFixLen.hpp
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (ffft_test_TestHelperFixLen_CURRENT_CODEHEADER)
	#error Recursive inclusion of TestHelperFixLen code header.
#endif
#define	ffft_test_TestHelperFixLen_CURRENT_CODEHEADER

#if ! defined (ffft_test_TestHelperFixLen_CODEHEADER_INCLUDED)
#define	ffft_test_TestHelperFixLen_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"ffft/test/conf.h"

#include	"ffft/test/TestAccuracy.h"
#if defined (ffft_test_SPEED_TEST_ENABLED)
	#include	"ffft/test/TestSpeed.h"
#endif



namespace ffft
{
namespace test
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <int L>
void	TestHelperFixLen <L>::perform_test_accuracy (int &ret_val)
{
   if (ret_val == 0)
   {
		FftType			fft;
      ret_val = TestAccuracy <FftType>::perform_test_single_object (fft);
   }
}



template <int L>
void	TestHelperFixLen <L>::perform_test_speed (int &ret_val)
{
#if defined (ffft_test_SPEED_TEST_ENABLED)

   if (ret_val == 0)
   {
		FftType			fft;
      ret_val = TestSpeed <FftType>::perform_test_single_object (fft);
   }

#endif
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}	// namespace test
}	// namespace ffft



#endif	// ffft_test_TestHelperFixLen_CODEHEADER_INCLUDED

#undef ffft_test_TestHelperFixLen_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
