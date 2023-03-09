/*
    CTS256A-AL2 - Console Proxy.

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

