/*
    CTS256A-AL2 - Console Debugger.

    Created by Michel Bernard (michel_bernard@hotmail.com)
    - <http://www.github.com/GmEsoft/SP0256_CTS256A-AL2>
    Copyright (c) 2023 Michel Bernard.
    All rights reserved.


    This file is part of SP0256_CTS256A-AL2.

    SP0256_CTS256A-AL2 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SP0256_CTS256A-AL2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SP0256_CTS256A-AL2.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "SystemConsole.h"
#include "DebugHelper_I.h"

class ConsoleDebugger
{
public:
	ConsoleDebugger( SystemConsole &systemConsole, DebugHelper_I &helper, Mode &mode )
		: systemConsole_( systemConsole ), helper_( helper ), mode_( mode )
		, breakPoint_( 0xFFFF ), breakOn_( false ), lines_( 25 )
	{
		init();
	}

	virtual ~ConsoleDebugger(void)
	{
		uninit();
	}

	void init();

	void uninit();

	void display();

	void displayLast( uint lastpc );

	void doCommand( int c );

	void setBreakPoint( uint breakPoint )
	{
		breakPoint_ = breakPoint;
	}

	uint getBreakPoint()
	{
		return breakPoint_;
	}

	void setBreakOn( bool breakOn )
	{
		breakOn_ = breakOn;
	}

	bool isBreakOn()
	{
		return breakOn_;
	}

	uint getRetSP()
	{
		return retSP_;
	}

	void setLines( uint lines )
	{
		lines_ = lines;
	}

	friend void sigbreakhandler(int s);
private:
	SystemConsole	&systemConsole_;
	DebugHelper_I	&helper_;
	Mode			&mode_;
	uint			breakPoint_;
	bool			breakOn_;
	uint			retSP_;
	uint			lines_;
};

