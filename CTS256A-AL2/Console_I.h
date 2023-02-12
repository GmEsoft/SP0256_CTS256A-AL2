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
