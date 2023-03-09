/*
    CTS256A-AL2 - Console Debugger.

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

#pragma warning(disable:4996)	// warning C4996: '%0': This function or variable may be unsafe.

#include "ConsoleDebugger.h"
#include "Memory_I.h"

#include <signal.h>
#include <stdio.h>

static ConsoleDebugger *consoleDebugger = 0;

static void sigbreakhandler(int s)
{
	if ( consoleDebugger )
		consoleDebugger->mode_.setMode( MODE_STOP );
    return;
}



static int regLines = 0;
static uint pc = 0, nextpc = 0;
static uint lastBreakPoint_ = 0xFFFF;
static ushort hexptr = 0;

static void hexDumpLine( Console_I &con, Memory_I &mem, ushort p )
{
	con.printf( "%04X :", p );
	for ( int j=0; j<16; ++j )
	{
		if ( !(j&7) )
			con.putch( ' ' );
		con.printf( "%02X ", mem.read( p++ ) );
	}
	p -= 16;
	for ( int j=0; j<16; ++j )
	{
		if ( !(j&7) )
			con.putch( ' ' );
		uchar c=mem.read( p++ );
		con.putch( ( c<' ' || c>0x7F ) ? '.' : c );
	}
	con.putch( '\n' );
}

static void hexDump( Console_I &con, Memory_I &mem, ushort p )
{
	for ( int i=0; i<16; ++i )
	{
		hexDumpLine( con, mem, p );
		p +=16 ;
	}
}

static void showHelp( Console_I &con )
{
	con.puts(
		"\n I = Instruction Step"
		"\n C = CALL Step"
		"\n G = Go"
		"\n E = Exec until $BREAK"
		"\n B = Exec until breakpoint"
		"\n X = Exec until RET"
		"\n S = Show next lines of disassembly"
		"\n R = Show reg names"
		"\n M = Registers indirect dump"
		"\n H = Hex dump current page"
		"\n ; = Hex dump next page"
		"\n - = Hex dump prev page"
		"\n . = Hex dump 16 pages forward"
		"\n _ = Hex dump 16 pages backward"
		"\n F = Font set"
		);
}

static void showCharSet( Console_I &con )
{
	con.puts( "\n   " );
	for (int i=0; i<16; ++i)
		con.printf( "%02X ", i );
	con.putch( '\n' );
	for (int i=0; i<16; ++i )
	{
		con.printf( "%02X  ", i<<4 );
		for (int j=0; j<16; ++j )
		{
			int c=(i<<4)+j;
			if ( !c || c>=0x07 && c<=0x0A || c==0x0C || c==0x0D || c>=0x1B && c<=0x1F )
				c=' ';
			con.putch( c );
			con.puts( "  " );
		}
		con.putch( '\n' );
	}
}



void ConsoleDebugger::init()
{
	consoleDebugger = this;
	signal( SIGBREAK, sigbreakhandler );
}

void ConsoleDebugger::uninit()
{
	consoleDebugger = 0;
}

void ConsoleDebugger::display()
{
	if ( !regLines )
	{
		systemConsole_.println();
		helper_.printTabSourceWidth( systemConsole_ );
		helper_.printRegNamesLine( systemConsole_ );
	}

	regLines = ( regLines + 1 ) % 16;

	nextpc = helper_.getPC();
	systemConsole_.println();
	helper_.printSource( systemConsole_, nextpc );
	helper_.printRegsLine( systemConsole_ );
}

void ConsoleDebugger::displayLast( uint lastpc )
{
	regLines = 0;
	systemConsole_.println();
	helper_.printSource( systemConsole_, lastpc );
}

void ConsoleDebugger::doCommand( int c )
{
	pc = helper_.getPC();
	if ( c == 'G' ) // GO
	{
		breakOn_ = false;
		regLines = 0;
		mode_.setMode( MODE_RUN );
		systemConsole_.putch( '\n' );
	}
	else if ( c == 'R' ) // SHOW REGISTER NAMES
	{
		regLines = 0;
	}
	else if ( c == 'B' ) // BREAKPOINT
	{
		char buf[31];
		systemConsole_.puts( "\nBreakpoint: " );
		char *str = systemConsole_.gets( buf, sizeof buf );
		if ( !str )
		{
			regLines = 0;
			return;
		}
		if ( !*buf )
		{
			breakPoint_ = lastBreakPoint_;;
			systemConsole_.printf( "Breakpoint: %04X\n", breakPoint_ );
		}
		else
		{
			sscanf( buf, "%x", &breakPoint_ );
			lastBreakPoint_ = breakPoint_;
		}
		mode_.setMode( MODE_RUN );
	}
	else if ( c == 'C' ) // CALL STEP
	{
		if ( helper_.isCall( pc ) )
		{
			breakPoint_ = nextpc;
			mode_.setMode( MODE_RUN );
			systemConsole_.putch( '\n' );
		}
		else
		{
			helper_.sim();
		}
	}
	else if ( c == 'X' ) // UNTIL RET
	{
		retSP_ = helper_.getSP();
		mode_.setMode( MODE_RET );
		systemConsole_.putch( '\n' );
	}
	else if ( c == 'E' ) // EXEC UNTIL $BREAK
	{
		regLines = 0;
		mode_.setMode( MODE_RUN );
		systemConsole_.putch( '\n' );
	}
	else if ( c == 'H' ) // HEX DUMP
	{
		regLines = 0;
		systemConsole_.putch( '\n' );
		hexDump( systemConsole_, helper_.getData(), hexptr );
	}
	else if ( c == ';' ) // HEX DUMP NEXT PAGE
	{
		hexptr += 0x0100;
		regLines = 0;
		systemConsole_.putch( '\n' );
		hexDump( systemConsole_, helper_.getData(), hexptr );
	}
	else if ( c == '-' ) // HEX DUMP PREV PAGE
	{
		hexptr -= 0x0100;
		regLines = 0;
		systemConsole_.putch( '\n' );
		hexDump( systemConsole_, helper_.getData(), hexptr );
	}
	else if ( c == '.' ) // HEX DUMP 16 PAGES FORWARD
	{
		hexptr += 0x1000;
		regLines = 0;
		systemConsole_.putch( '\n' );
		hexDump( systemConsole_, helper_.getData(), hexptr );
	}
	else if ( c == '_' ) // HEX DUMP 16 PAGES BACKWARD
	{
		hexptr -= 0x1000;
		regLines = 0;
		systemConsole_.putch( '\n' );
		hexDump( systemConsole_, helper_.getData(), hexptr );
	}
	else if ( c == 'M' ) // HEX DUMP PTRS
	{
		regLines = 0;
		systemConsole_.putch( '\n' );
		for ( int i=0; ; ++i )
		{
			uint ptr;
			const char *ptrName = helper_.getPointer( i, ptr );
			if ( !ptrName )
				break;

			systemConsole_.printf( "%s=", ptrName );
			hexDumpLine( systemConsole_, helper_.getData(), ptr );
		}
	}
	else if ( c == ' ' || c == 'I' ) // INSTRUCTION STEP
	{
		helper_.sim();
	}
	else if ( c == 'F' ) // CHAR SET
	{
		showCharSet( systemConsole_ );
		regLines = 0;
	}
	else if ( c == 'S' ) // SHOW SOURCE
	{
		uint pc = helper_.getPC();
		helper_.getSource( pc );
		for ( uint i=3; i<lines_; ++i )
		{
			systemConsole_.println();
			helper_.printSource( systemConsole_, pc );
		}
		regLines = 0;
	}
	else if ( c == '?' ) // HELP
	{
		showHelp( systemConsole_ );
		regLines = 0;
	}

}
