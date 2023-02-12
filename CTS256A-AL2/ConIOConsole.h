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
	bool autolf_;
	int backspace_;
};

