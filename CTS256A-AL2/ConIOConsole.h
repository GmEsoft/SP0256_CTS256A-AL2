/*
    CTS256A-AL2 - ConIO Console.

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
#include "Mode.h"

class ConIOConsole :
	public Console_I
{
public:
	ConIOConsole(void);

	~ConIOConsole(void)
	{
	}

	virtual int kbhit();
	virtual int getch();
	virtual int poll();
	virtual int ungetch( int ch );
	virtual int putch( int ch );
	virtual int puts( const char *str );
	virtual char *gets( char *str, size_t size );
	virtual int vprintf( const char *str, va_list args );

	void setAutoLf( bool autolf )
	{
		autolf_ = autolf;
	}

	void setBackSpace( int bs )
	{
		backspace_ = bs;
	}

private:
	int ch_;
	int backspace_;
	bool autolf_;
};

