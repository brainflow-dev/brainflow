/*****************************************************************************

        StopWatch.hpp
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (stopwatch_StopWatch_CURRENT_CODEHEADER)
	#error Recursive inclusion of StopWatch code header.
#endif
#define	stopwatch_StopWatch_CURRENT_CODEHEADER

#if ! defined (stopwatch_StopWatch_CODEHEADER_INCLUDED)
#define	stopwatch_StopWatch_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace stopwatch
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	StopWatch::start ()
{
	_nbr_laps = 0;
	_ccc.start ();
}



void	StopWatch::stop_lap ()
{
	_ccc.stop_lap ();
	++ _nbr_laps;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}	// namespace stopwatch



#endif	// stopwatch_StopWatch_CODEHEADER_INCLUDED

#undef stopwatch_StopWatch_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
