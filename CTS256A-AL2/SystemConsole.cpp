/*
    CTS256A-AL2 - System Console.

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

#include "SystemConsole.h"

#define NEW 1

int SystemConsole::checkch( int ch )
{
	if ( ch == -68 ) // F10 == stop
	{
		if ( system_ )
			system_->stop();
	}
	else if ( ch == -93 ) // Sh-F10 = exit
	{
		if ( system_ )
			system_->exit();
	}
	else if ( ch == -84 ) // Sh-F1 = reset
	{
		if ( system_ )
			system_->reset();
	}
	else
		return ch;
	return -1;
}

int SystemConsole::kbhit()
{
	if ( !kbCount_-- && console_ )
	{
		kbCount_ = kbReload_;
		int ret = console_->kbhit();
		if ( ret )
		{
			ret = checkch( console_->poll() );
			if ( ret == -1 )
				console_->getch();
			else
				kbHit_ = 1;
		}
		else if ( ch_ )
		{
			kbHit_ = 1;
		}
	}

	return kbHit_;
}

int SystemConsole::getch()
{
	int ret = ch_;
	ch_ = 0;
	kbHit_ = 0;

	if ( ret )
	{
		kbCount_ = 0;
		return ret;
	}

	if ( console_ )
	{
		ret = checkch( console_->getch() );
		if ( ret )
		{
			kbCount_ = 0;
			if ( ret != -1 )
				return ret;
		}
	}
	return 0;
}

int SystemConsole::poll()
{
	if ( console_ )
		return console_->poll();
	return 0;
}

int SystemConsole::ungetch( int ch )
{
	if ( console_ )
	{
		if ( !console_->kbhit() )
			return console_->ungetch( ch );
	}
	return ch_ = ch;
}

int SystemConsole::putch( int ch )
{
	if ( console_ )
		return console_->putch( ch );
	return ch;
}

int SystemConsole::puts( const char *str )
{
	if ( console_ )
		return console_->puts( str );;
	return 0;
}

char *SystemConsole::gets( char *str, size_t size )
{
	if ( console_ )
		return console_->gets( str, size );
	return 0;
}

int SystemConsole::vprintf( const char *str, va_list args )
{
	if ( console_ )
		return console_->vprintf( str, args );
	return 0;
}

