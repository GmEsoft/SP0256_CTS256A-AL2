#pragma once

#include "Console_I.h"

class ConsoleProxy :
	public Console_I
{
public:

	ConsoleProxy(void) : console_( 0 )
	{
	}

	virtual ~ConsoleProxy(void)
	{
	}

	void setConsole( Console_I *console )
	{
		console_ = console;
	}

	int putch( int ch )
	{
		return console_ ? console_->putch( ch ) : 0;
	}

	int kbhit()
	{
		return console_ ? console_->kbhit() : 0;
	}

	int getch()
	{
		return console_ ? console_->getch() : 0;
	}

	int poll()
	{
		return console_ ? console_->poll() : 0;
	}

	int ungetch( int ch )
	{
		return console_ ? console_->ungetch( ch ) : 0;
	}

	int puts( const char *str )
	{
		if ( console_ )
			return console_->puts( str );;
		return 0;
	}

	char *gets( char *str, size_t size )
	{
		if ( console_ )
			return console_->gets( str, size );
		return 0;
	}

	int vprintf( const char *str, va_list args )
	{
		if ( console_ )
			return console_->vprintf( str, args );
		return 0;
	}

protected:
	Console_I		*console_;
};

