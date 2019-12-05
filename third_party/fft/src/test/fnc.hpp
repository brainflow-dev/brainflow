/*****************************************************************************

        fnc.hpp
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (ffft_test_fnc_CURRENT_CODEHEADER)
	#error Recursive inclusion of fnc code header.
#endif
#define	ffft_test_fnc_CURRENT_CODEHEADER

#if ! defined (ffft_test_fnc_CODEHEADER_INCLUDED)
#define	ffft_test_fnc_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace ffft
{
namespace test
{



template <class T>
T	limit (const T &x, const T &inf, const T &sup)
{
	assert (! (sup < inf));

	return ((x < inf) ? inf : ((sup < x) ? sup : x));
}



}	// namespace test
}	// namespace ffft



#endif	// ffft_test_fnc_CODEHEADER_INCLUDED

#undef ffft_test_fnc_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
