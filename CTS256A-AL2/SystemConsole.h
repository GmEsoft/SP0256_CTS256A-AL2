#pragma once

#include "Console_I.h"

#include "Mode.h"
#include "System_I.h"


// Console decorator for system control

class SystemConsole :
	public Console_I
{
public:
	SystemConsole(void) : system_( 0 ), ch_( 0 ), console_( 0 ), kbReload_( 0x100 ), kbCount_( 0 ), kbHit_( 0 )
	{
	}

	virtual ~SystemConsole(void)
	{
	}

	void setConsole( Console_I *console )
	{
		console_ = console;
	}

	void setSystem( System_I *system )
	{
		system_ = system;
	}

	void setKbReload( int kbReload )
	{
		kbReload_ = kbReload;
	}

	// Console API implementation
	int kbhit();

	int getch();

	int poll();

	int ungetch( int ch );

	int putch( int ch );

	int puts( const char * str );
	
	char *gets( char * str, size_t size );
	
	int vprintf( const char* format, va_list args );

private:
	int checkch( int ch );

	int			ch_;
	Console_I	*console_;
	System_I	*system_;
	int			kbCount_;
	int			kbReload_;
	int			kbHit_;
};
