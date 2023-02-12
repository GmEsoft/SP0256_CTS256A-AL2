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

