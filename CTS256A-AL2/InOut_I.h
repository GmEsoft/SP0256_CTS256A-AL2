#pragma once

// I/O API

#include "runtime.h"

class InOut_I
{
public:
	// out char
	virtual uchar out( ushort addr, uchar data ) = 0;

	// in char
	virtual uchar in( ushort addr ) = 0;

	// destructor
	virtual ~InOut_I()
	{
	}
};
