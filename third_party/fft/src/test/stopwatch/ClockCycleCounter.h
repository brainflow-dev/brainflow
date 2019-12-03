/*****************************************************************************

        ClockCycleCounter.h
        By Laurent de Soras

Instrumentation class, for accurate time interval measurement. You may have
to modify the implementation to adapt it to your system and/or compiler.

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (stopwatch_ClockCycleCounter_HEADER_INCLUDED)
#define	stopwatch_ClockCycleCounter_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"def.h"
#include	"Int64.h"



namespace stopwatch
{



class ClockCycleCounter
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

						ClockCycleCounter ();

	stopwatch_FORCEINLINE void
						start ();
	stopwatch_FORCEINLINE void
						stop_lap ();
	Int64				get_time_total () const;
	Int64				get_time_best_lap () const;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	void				compute_clk_mul ();
	void				compute_measure_time_total ();
	void				compute_measure_time_lap ();

	static void		spend_time ();
	static stopwatch_FORCEINLINE Int64
						read_clock_counter ();

	Int64				_start_time;
	Int64				_state;
	Int64				_best_score;

	static Int64	_measure_time_total;
	static Int64	_measure_time_lap;
	static int		_clk_mul;
	static bool		_init_flag;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						ClockCycleCounter (const ClockCycleCounter &other);
	ClockCycleCounter &
						operator = (const ClockCycleCounter &other);
	bool				operator == (const ClockCycleCounter &other);
	bool				operator != (const ClockCycleCounter &other);

};	// class ClockCycleCounter



}	// namespace stopwatch



#include	"ClockCycleCounter.hpp"



#endif	// stopwatch_ClockCycleCounter_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
