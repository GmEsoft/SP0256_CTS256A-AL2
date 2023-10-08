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

#include "ConIOConsole.h"

#include <signal.h>
#include <conio.h>

#if __cplusplus < 201103L
#	define noexcept throw()
#endif

static void siginthandler (int) noexcept
{
    signal(SIGINT,siginthandler);   // reinstall signal handler
	_ungetch( 3 );
    return;
}

ConIOConsole::ConIOConsole() : autolf_( false ), backspace_( 0x08 ), ch_( 0 )
{
	signal( SIGINT, siginthandler );
}

int ConIOConsole::kbhit()
{
	int ret = ch_ || _kbhit();
#ifdef _DEBUG
	if ( ret )
		ret = ret;
#endif

	return ret;
}

int ConIOConsole::getch()
{
	int c = ch_ ? ch_ : _getch();

	ch_ = 0;

	if ( !c || c == 0xE0 )
		c = -_getch();

	if ( c == 0x08 )
		c = backspace_;

	return c;
}

int ConIOConsole::poll()
{
	if ( !_kbhit() )
		return 0;

	int c = _getch();

	if ( !c || c == 0xE0 )
		c = -_getch();

	if ( c == 0x08 )
		c = backspace_;

	if ( c )
		ch_ = c;
	return c;
}

int ConIOConsole::ungetch( int ch )
{
	return ch_ = ch;
}

int ConIOConsole::putch( int ch )
{
	int ret = _putch( ch );
	if ( autolf_ && ch == 0x0D )
		ret = _putch( 0x0A );
	return ret;
}

int ConIOConsole::puts( const char *str )
{
	return ::_cputs( str );
}

//extern "C" intptr_t _coninpfh;

char *ConIOConsole::gets( char *str, size_t size )
{
#if 0
	char *ret = ::_cgets( str ); // How to flush console input stream ?
	//HANDLE inHandle = ::GetStdHandle( CONSOLE );
	//HANDLE inHandle = (HANDLE)0x13;//CreateFile(L"CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	//::FlushConsoleInputBuffer( inHandle );
	return ret;
#else
	int len = int( size - 1 );
	int p = 0;
	int c = this->getch();

	while ( c != '\r' )
	{
		if ( c == 0x18 || c == 0x1B )						// Ctrl-X or Esc
		{
			for ( ; p; --p )
				this->puts( "\x08\x20\x08" );
			p = 0;
			if ( c == 0x1B )								// Esc
			{
				c = 0;
				break;
			}
		}
		else if ( ( c == 8 || c == 0x7F ) && p > 0 )		// BS or Del
		{
			--p;
			this->puts( "\x08\x20\x08" );
		}
		else if ( c >= ' ' && c < 0x7F && p < len )			// Character
		{
			str[p] = (char)c;
			++p;
			this->putch( c );
		}
		else
		{
			this->printf( " [%04X]\x08\x08\x08\x08\x08\x08\x08", c & 0xFFFF );
		}
		c = this->getch();
	}

	if ( c )
		this->putch( c );
	str[p] = 0;

	return str;
#endif
}

int ConIOConsole::vprintf( const char *str, va_list args )
{
	return ::_vcprintf( str, args );
}
