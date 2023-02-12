#pragma once

#include "Sleeper_I.h"

#include <windows.h>

class Win32Sleeper :
	public Sleeper_I
{
public:

	Win32Sleeper(void)
	{
	}

	virtual ~Win32Sleeper(void)
	{
	}

	void sleep( ulong millis )
	{
		Sleep( millis );
	}
};

