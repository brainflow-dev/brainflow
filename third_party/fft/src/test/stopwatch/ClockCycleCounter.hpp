/*****************************************************************************

        ClockCycleCounter.hpp
        By Laurent de Soras

Please complete the definitions according to your compiler/architecture.
It's not a big deal if it's not possible to get the clock count...

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (stopwatch_ClockCycleCounter_CURRENT_CODEHEADER)
	#error Recursive inclusion of ClockCycleCounter code header.
#endif
#define	stopwatch_ClockCycleCounter_CURRENT_CODEHEADER

#if ! defined (stopwatch_ClockCycleCounter_CODEHEADER_INCLUDED)
#define	stopwatch_ClockCycleCounter_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"fnc.h"

#include	<climits>



namespace stopwatch
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*
==============================================================================
Name: start
Description:
	Starts the counter.
Throws: Nothing
==============================================================================
*/

void	ClockCycleCounter::start ()
{
	_best_score = (static_cast <Int64> (1) << (sizeof (Int64) * CHAR_BIT - 2));
	const Int64		start_clock = read_clock_counter ();
	_start_time = start_clock;
	_state = start_clock - _best_score;
}



/*
==============================================================================
Name: stop_lap
Description:
	Captures the current time and updates the smallest duration between two
	consecutive calls to stop_lap() or the latest start().
	start() must have been called at least once before calling this function.
Throws: Nothing
==============================================================================
*/

void	ClockCycleCounter::stop_lap ()
{
	const Int64		end_clock = read_clock_counter ();
	_best_score = min (end_clock - _state, _best_score);
	_state = end_clock;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



Int64	ClockCycleCounter::read_clock_counter ()
{
	register Int64		clock_cnt;

#if defined (_MSC_VER)

	__asm
	{
		lea				edi, clock_cnt
		rdtsc
		mov				[edi    ], eax
		mov				[edi + 4], edx
	}

#elif defined (__GNUC__) && defined (__i386__)

	__asm__ __volatile__ ("rdtsc" : "=A" (clock_cnt));

#elif (__MWERKS__) && defined (__POWERPC__) 
	
	asm
	{
	loop:
		mftbu			clock_cnt@hiword
		mftb			clock_cnt@loword
		mftbu			r5
		cmpw			clock_cnt@hiword,r5
		bne loop
	}
	
#endif

	return (clock_cnt);
}



}	// namespace stopwatch



#endif	// stopwatch_ClockCycleCounter_CODEHEADER_INCLUDED

#undef stopwatch_ClockCycleCounter_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
