/*****************************************************************************

        TestWhiteNoiseGen.h
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (ffft_test_TestWhiteNoiseGen_HEADER_INCLUDED)
#define	ffft_test_TestWhiteNoiseGen_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace ffft
{
namespace test
{



template <class DT>
class TestWhiteNoiseGen
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef	DT	DataType;

						TestWhiteNoiseGen ();
	virtual			~TestWhiteNoiseGen () {}

	void				generate (DataType data_ptr [], long len);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	typedef	unsigned long	StateType;

	StateType		_rand_state;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						TestWhiteNoiseGen (const TestWhiteNoiseGen &other);
	TestWhiteNoiseGen &
						operator = (const TestWhiteNoiseGen &other);
	bool				operator == (const TestWhiteNoiseGen &other);
	bool				operator != (const TestWhiteNoiseGen &other);

};	// class TestWhiteNoiseGen



}	// namespace test
}	// namespace ffft



#include	"ffft/test/TestWhiteNoiseGen.hpp"



#endif	// ffft_test_TestWhiteNoiseGen_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
