/*****************************************************************************

        Int64.h
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (stopwatch_Int64_HEADER_INCLUDED)
#define	stopwatch_Int64_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace stopwatch
{


#if defined (_MSC_VER)

	typedef	__int64	Int64;

#elif defined (__MWERKS__) || defined (__GNUC__)

	typedef	long long	Int64;

#elif defined (__BEOS__)

	typedef	int64	Int64;

#else

	#error No 64-bit integer type defined for this compiler !

#endif


}	// namespace stopwatch



#endif	// stopwatch_Int64_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
