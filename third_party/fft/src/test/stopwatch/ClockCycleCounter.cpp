/*****************************************************************************

        ClockCycleCounter.cpp
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (_MSC_VER)
	#pragma warning (1 : 4130) // "'operator' : logical operation on address of string constant"
	#pragma warning (1 : 4223) // "nonstandard extension used : non-lvalue array converted to pointer"
	#pragma warning (1 : 4705) // "statement has no effect"
	#pragma warning (1 : 4706) // "assignment within conditional expression"
	#pragma warning (4 : 4786) // "identifier was truncated to '255' characters in the debug information"
	#pragma warning (4 : 4800) // "forcing value to bool 'true' or 'false' (performance warning)"
	#pragma warning (4 : 4355) // "'this' : used in base member initializer list"
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"ClockCycleCounter.h"

#include	<cassert>



namespace stopwatch
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*
==============================================================================
Name: ctor
Description:
	The first object constructed initialise global data. This first
	construction may be a bit slow.
Throws: Nothing
==============================================================================
*/

ClockCycleCounter::ClockCycleCounter ()
:	_start_time (0)
,	_state (0)
,	_best_score (-1)
{
	if (! _init_flag)
	{
		// Should be executed in this order
		compute_clk_mul ();
		compute_measure_time_total ();
		compute_measure_time_lap ();

		// Restores object state
		_start_time = 0;
		_state      = 0;
		_best_score = -1;

		_init_flag = true;
	}
}



/*
==============================================================================
Name: get_time_total
Description:
	Gives the time elapsed between the latest stop_lap() and start() calls.
Returns:
	The duration, in clock cycles.
Throws: Nothing
==============================================================================
*/

Int64	ClockCycleCounter::get_time_total () const
{
	const Int64		duration = _state - _start_time;
	assert (duration >= 0);

	const Int64		t = max (
		duration - _measure_time_total,
		static_cast <Int64> (0)
	);

	return (t * _clk_mul);
}



/*
==============================================================================
Name: get_time_best_lap
Description:
	Gives the smallest time between two consecutive stop_lap() or between
	the stop_lap() and start(). The value is reset by a call to start().
	Call this function only after a stop_lap().
	The time is amputed from the duration of the stop_lap() call itself.
Returns:
	The smallest duration, in clock cycles.
Throws: Nothing
==============================================================================
*/

Int64	ClockCycleCounter::get_time_best_lap () const
{
	assert (_best_score >= 0);

	const Int64		t1 = max (
		_best_score - _measure_time_lap,
		static_cast <Int64> (0)
	);
	const Int64		t = min (t1, get_time_total ());

	return (t * _clk_mul);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



#if defined (__MACOS__)

static inline double	stopwatch_ClockCycleCounter_get_time_s ()
{
	const Nanoseconds	ns = AbsoluteToNanoseconds (UpTime ());

	return (ns.hi * 4294967296e-9 + ns.lo * 1e-9);
}

#endif	// __MACOS__



/*
==============================================================================
Name: compute_clk_mul
Description:
	This function, only for PowerPC/MacOS computers, computes the multiplier
	required to deduce clock cycles from the internal counter.
Throws: Nothing
==============================================================================
*/

void	ClockCycleCounter::compute_clk_mul ()
{
	assert (! _init_flag);

#if defined (__MACOS__)

	long				clk_speed_mhz = CurrentProcessorSpeed ();
	const Int64		clk_speed =
		static_cast <Int64> (clk_speed_mhz) * (1000L*1000L);

	const double	start_time_s = stopwatch_ClockCycleCounter_get_time_s ();
	start ();

	const double	duration = 0.01;	// Seconds
	while (stopwatch_ClockCycleCounter_get_time_s () - start_time_s < duration)
	{
		continue;
	}

	const double	stop_time_s = stopwatch_ClockCycleCounter_get_time_s ();
	stop ();

	const double	diff_time_s = stop_time_s - start_time_s;
	const double	nbr_cycles = diff_time_s * static_cast <double> (clk_speed);

	const Int64		diff_time_c = _state - _start_time;
	const double	clk_mul = nbr_cycles / static_cast <double> (diff_time_c);

	_clk_mul = round_int (clk_mul);

#endif	// __MACOS__
}



void	ClockCycleCounter::compute_measure_time_total ()
{
	start ();
	spend_time ();

	Int64				best_result = 0x7FFFFFFFL;	// Should be enough
	long				nbr_tests = 100;
	for (long cnt = 0; cnt < nbr_tests; ++cnt)
	{
		start ();
		stop_lap ();
		const Int64		duration = _state - _start_time;
		best_result = min (best_result, duration);
	}

	_measure_time_total = best_result;
}



/*
==============================================================================
Name: compute_measure_time_lap
Description:
	Computes the duration of one stop_lap() call and store it. It will be used
	later to get the real duration of the measured operation (by substracting
	the measurement duration).
Throws: Nothing
==============================================================================
*/

void	ClockCycleCounter::compute_measure_time_lap ()
{
	start ();
	spend_time ();

	long				nbr_tests = 10;
	for (long cnt = 0; cnt < nbr_tests; ++cnt)
	{
		stop_lap ();
		stop_lap ();
		stop_lap ();
		stop_lap ();
	}

	_measure_time_lap = _best_score;
}



void	ClockCycleCounter::spend_time ()
{
	const Int64		nbr_clocks = 500;	// Number of clock cycles to spend

	const Int64		start = read_clock_counter ();
	Int64				current;

	do
	{
		current = read_clock_counter ();
	}
	while ((current - start) * _clk_mul < nbr_clocks);
}



Int64	ClockCycleCounter::_measure_time_total = 0;
Int64	ClockCycleCounter::_measure_time_lap = 0;
int	ClockCycleCounter::_clk_mul = 1;
bool	ClockCycleCounter::_init_flag = false;


}	// namespace stopwatch



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
