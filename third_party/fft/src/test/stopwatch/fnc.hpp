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



#if defined (stopwatch_fnc_CURRENT_CODEHEADER)
	#error Recursive inclusion of fnc code header.
#endif
#define	stopwatch_fnc_CURRENT_CODEHEADER

#if ! defined (stopwatch_fnc_CODEHEADER_INCLUDED)
#define	stopwatch_fnc_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	<cassert>
#include	<cmath>

namespace std {}



namespace stopwatch
{



template <typename T>
inline T	min (T a, T b)
{
	return ((a < b) ? a : b);
}



template <typename T>
inline T	max (T a, T b)
{
	return ((b < a) ? a : b);
}



int	round_int (double x)
{
	using namespace std;

	return (static_cast <int> (floor (x + 0.5)));
}



}	// namespace stopwatch



#endif	// stopwatch_fnc_CODEHEADER_INCLUDED

#undef stopwatch_fnc_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
