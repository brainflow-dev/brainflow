/*****************************************************************************

        StopWatch.h
        By Laurent de Soras

Utility class based on ClockCycleCounter to measure the unit time of a
repeated operation.

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (stopwatch_StopWatch_HEADER_INCLUDED)
#define	stopwatch_StopWatch_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"ClockCycleCounter.h"



namespace stopwatch
{



class StopWatch
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

						StopWatch ();

	stopwatch_FORCEINLINE void
						start ();
	stopwatch_FORCEINLINE void
						stop_lap ();

	double			get_time_total (Int64 nbr_op) const;
	double			get_time_best_lap (Int64 nbr_op) const;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	ClockCycleCounter
						_ccc;
	Int64				_nbr_laps;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						StopWatch (const StopWatch &other);
	StopWatch &		operator = (const StopWatch &other);
	bool				operator == (const StopWatch &other);
	bool				operator != (const StopWatch &other);

};	// class StopWatch



}	// namespace stopwatch



#include	"StopWatch.hpp"



#endif	// stopwatch_StopWatch_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
