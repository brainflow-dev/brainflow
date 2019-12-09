/*****************************************************************************

        TestWhiteNoiseGen.hpp
        By Laurent de Soras

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if defined (ffft_test_TestWhiteNoiseGen_CURRENT_CODEHEADER)
	#error Recursive inclusion of TestWhiteNoiseGen code header.
#endif
#define	ffft_test_TestWhiteNoiseGen_CURRENT_CODEHEADER

#if ! defined (ffft_test_TestWhiteNoiseGen_CODEHEADER_INCLUDED)
#define	ffft_test_TestWhiteNoiseGen_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	<cstddef>



namespace ffft
{
namespace test
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <class DT>
TestWhiteNoiseGen <DT>::TestWhiteNoiseGen ()
:	_rand_state (0)
{
	_rand_state = StateType (reinterpret_cast <ptrdiff_t> (this));
}



template <class DT>
void	TestWhiteNoiseGen <DT>::generate (DataType data_ptr [], long len)
{
	assert (data_ptr != 0);
	assert (len > 0);

	const DataType	one = static_cast <DataType> (1);
	const DataType	mul = one / static_cast <DataType> (0x80000000UL);

	long				pos = 0;
	do
	{
		const DataType	x = static_cast <DataType> (_rand_state & 0xFFFFFFFFUL);
		data_ptr [pos] = x * mul - one;

		_rand_state = _rand_state * 1234567UL + 890123UL;

		++ pos;
	}
	while (pos < len);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}	// namespace test
}	// namespace ffft



#endif	// ffft_test_TestWhiteNoiseGen_CODEHEADER_INCLUDED

#undef ffft_test_TestWhiteNoiseGen_CURRENT_CODEHEADER



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
