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

