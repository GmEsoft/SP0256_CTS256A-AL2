#include "ConIOConsole.h"

#include <signal.h>
#include <conio.h>

static void siginthandler (int s)
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
