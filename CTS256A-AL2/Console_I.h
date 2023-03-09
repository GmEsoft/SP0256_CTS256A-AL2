/*
    CTS256A-AL2 - Console API.

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

#include "Mode.h"

#include <stdarg.h>

// STANDARD CONSOLE API

class Console_I
{
public:

	virtual int kbhit() = 0;
	virtual int getch() = 0;
	virtual int poll() = 0;
	virtual int ungetch( int ch ) = 0;
	virtual int putch( int ch ) = 0;
	virtual int puts( const char * str ) = 0;
	virtual char *gets( char *str, size_t size ) = 0;
	virtual int vprintf( const char *format, va_list args ) = 0;

	int printf( const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		int ret = vprintf( format, args );
		va_end( args );
		return ret;
	}

	int available()
	{
		return kbhit() ? 1 : 0;
	}

	char read()
	{
		return (char)getch();
	}
	
	int println()
	{
		return print( "\r\n" );
	}

	int println( const char* str )
	{
		int r = print( str );
        println();
        return r;
	}

	int print( char ch )
	{
		return putch( ch );
	}

	int print( const char* str )
	{
		return puts( str );
	}

	int print( int i, int base=10 )
	{
		return printf( base == 16 ? "%X" 
					 : base == 8 ? "%o"
					 : base == 2 ? "%b"
					 : "%d"
					 , i );
	}

	int print( long l, int base=10 )
	{
		return printf( base == 16 ? "%lX" 
					 : base == 8 ? "%lo"
					 : base == 2 ? "%lb"
					 : "%ld"
					 , l );
	}

	int println( int i, int base=10 )
	{
		int r = print( i, base );
		println();
		return r;
	}

	int println( long l, int base=10 )
	{
		int r = print( l, base );
		println();
		return r;
	}

	// destructor
	virtual ~Console_I()
	{
	}
};
