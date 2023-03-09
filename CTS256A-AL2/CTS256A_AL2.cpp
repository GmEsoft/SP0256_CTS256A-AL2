/*
    CTS256A-AL2 - CTS256A-AL2 System.

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

#include "CTS256A_AL2.h"

#include "TMS7000DebugHelper.h"
#include "ConsoleDebugger.h"

#include <stdio.h>
#include <ctype.h>

extern const uchar CTS256A_AL2_ROM[];

static const char * SP0256_labels[] = 
{
	"PA1",	"PA2",	"PA3",	"PA4",	"PA5",	"OY",	"AY",	"EH",
	"KK3",	"PP",	"JH",	"NN1",	"IH",	"TT2",	"RR1",	"AX",
	"MM",	"TT1",	"DH1",	"IY",	"EY",	"DD1",	"UW1",	"AO",
	"AA",	"YY2",	"AE",	"HH1",	"BB1",	"TH",	"UH",	"UW2",
	"AW",	"DD2",	"GG3",	"VV",	"GG1",	"SH",	"ZH",	"RR2",
	"FF",	"KK2",	"KK1",	"ZZ",	"NG",	"LL",	"WW",	"XR",
	"WH",	"YY1",	"CH",	"ER1",	"ER2",	"OW",	"DH2",	"SS",
	"NN2",	"HH2",	"OR",	"AR",	"YR",	"GG2",	"EL",	"BB2"
};


uchar CTS256A_AL2_Data_InOut::read( ushort addr )
{
	cpu_.trigIRQ( 0x02 ); // trig INT1

	if ( !eof_ ) 
	{
		if ( !initctr_ && ( bport_ & 0x01 ) )
			cpu_.trigIRQ( 0x08 ); // trig INT3
		if ( !--debugctr_ ) {
			cpu_.setMode( MODE_STOP );
			debugctr_ = DEBUG_CTR_RELOAD;
			cpu_.printf( "\nCTS256A_AL2 debugctr stopped at %04X\n", addr );
		}
	} 
	else if ( !--eofctr_ )
	{
		if ( debug_ )
			cpu_.printf( "\nCTS256A_AL2 eofctr stopped at %04X\n", addr );
		cpu_.setMode( debug_ ? MODE_STOP : MODE_EXIT );
	}

	// 0xF000-0xFFFF: CTS256A-AL2 ROM (in)
	if ( addr >= 0xF000 )
		return CTS256A_AL2_ROM[addr&0x0FFF];

	// 0x0200-0x0FFF: Parallel data (in)
	if ( addr < 0x1000 )
	{
		uchar c = istr_.get();
		if ( eof_ || istr_.eof() )
		{
			eof_ = true;
			eofctr_ = EOF_CTR_RELOAD;
			if ( verbose_ )
				cpu_.printf( " in: EOF\n" );
			return 0x0D;
		}

		if ( verbose_ )
			cpu_.printf( " in: %c\n", c );

		if ( echo_ )
			cpu_.putch( c );

		debugctr_ = DEBUG_CTR_RELOAD;
		return c;
	}

	// 0x1000-0x1FFF: UART Parameters (in)
	if ( addr < 0x2000 )
		return 0;

	// 0x2000-0x2FFF: SP0256 (out)
	if ( addr < 0x3000 )	
		return 0xFF;

	// 0x3000-0x37FF: RAM (in/out)
	if ( addr < 0x3800 )
		return ram_[addr & 0x07FF];

	// 0x3000-0xEFFF: RAM (in/out)
	if ( addr < 0xF000 )	
		return 0xFF;

	return 0xFF;
}

uchar CTS256A_AL2_Data_InOut::write( ushort addr, uchar data )
{
	// 0xF000-0xFFFF: CTS256A-AL2 ROM (in)
	if ( addr >= 0xF000 )
		return data;

	// 0x0200-0x0FFF: Parallel data (in)
	if ( addr < 0x1000 )	
		return data;

	// 0x1000-0x1FFF: UART Parameters (in)
	if ( addr < 0x2000 )
		return data;

	// 0x2000-0x2FFF: SP0256 (out)
	if ( addr < 0x3000 )
	{
		if ( eof_ )
		{
			if ( verbose_ )
				cpu_.printf( "%5d ", eofctr_ );
			eofctr_ = EOF_CTR_RELOAD;
		}

		if ( verbose_ )
			cpu_.printf( " SP0256: %02X=%s\n", data, data<0x40 ? SP0256_labels[data] : "**" );

		if ( !noOK_ || !initctr_ )
		{
			if ( mode_ == 'T' )
				ostr_ << " " << SP0256_labels[data];
			else
				ostr_.put( data | 0x40 );
		}

		if ( initctr_ )
			--initctr_;

		debugctr_ = DEBUG_CTR_RELOAD;

		return data;
	}

	// 0x3000-0x37FF: RAM (in/out)
	if ( addr < 0x3800 )
		return ram_[addr & 0x07FF] = data;

	// 0x3000-0xEFFF: RAM (in/out)
	if ( addr < 0xF000 )	
		return data;

	return data;
}

uchar CTS256A_AL2_Data_InOut::in( ushort addr )
{
	switch ( addr )
	{
	case 0x04:	// APORT (in)
		// 7 (80)	Delimiter: 0=CR - 1=any
		// 6 (40)	SCLK
		// 5 (20)	RXD
		// 4 (10)	RAM buffers: 0:internal(20in/26out) - 1:external(1792in/256out)
		// 3 (08)	serial cfg: 0:7N2  - 1:selectable
		// 2 (04)	m0 +	000:paral  - 001:50bd   - 010:110bd
		// 1 (02)	m1 |==> 011:300bd  - 100:1200bd - 101:2400bd
		// 0 (01)	m2 +    110:4800bd - 111:9600bd
		return 0x90;
	case 0x06:	// BPORT	(out) Port B data := xxxx xxxI (DSR/BUSY)
		// 7 (80)	CLKOUT
		// 6 (40)	ENABLE*
		// 5 (20)	R/W*
		// 4 (10)	ALATCH
		// 3 (08)	TXD
		// 2 (04)
		// 1 (02)
		// 0 (01)	DSR/BUSY
		return 0xFF;
	default:
		return 0xFF;
	}
	return 0xFF;
}

uchar CTS256A_AL2_Data_InOut::out( ushort addr, uchar data )
{
	switch ( addr )
	{
	case 0x04:	// APORT (in)
		// 7 (80)	Delimiter: 0=CR - 1=any
		// 6 (40)	SCLK
		// 5 (20)	RXD
		// 4 (10)	RAM buffers: 0:internal(20in/26out) - 1:external(1792in/256out)
		// 3 (08)	serial cfg: 0:7N2  - 1:selectable
		// 2 (04)	m0 +	000:paral  - 001:50bd   - 010:110bd
		// 1 (02)	m1 |==> 011:300bd  - 100:1200bd - 101:2400bd
		// 0 (01)	m2 +    110:4800bd - 111:9600bd
		return data;
	case 0x06:	// BPORT	(out) Port B data := xxxx xxxI (DSR/BUSY)
		// 7 (80)	CLKOUT
		// 6 (40)	ENABLE*
		// 5 (20)	R/W*
		// 4 (10)	ALATCH
		// 3 (08)	TXD
		// 2 (04)
		// 1 (02)
		// 0 (01)	DSR/BUSY
		bport_ = data;
		return data;
	default:
		return data;
	}
	return data;
}

void CTS256A_AL2_Data_InOut::setOption( uchar option, uint value )
{
	switch( option )
	{
	case 'E':
		echo_ = value != 0;
		break;
	case 'D':
		debug_ = value != 0;
		break;
	case 'V':
		verbose_ = value != 0;
		break;
	case 'N':
		noOK_ = value != 0;
		break;
	case 'M':
		mode_ = (uchar)value;
		break;
	default:
		cpu_.printf( "Unknown option %c=%d\n", option, value );
	}
}

uint CTS256A_AL2_Data_InOut::getOption( uchar option )
{
	switch( option )
	{
	case 'E':
		return echo_;
	case 'D':
		return debug_;
	case 'V':
		return verbose_;
	case 'N':
		return noOK_;
	case 'M':
		return mode_;
	default:
		cpu_.printf( "Unknown option %c\n", option );
		return 0;
	}
}


void CTS256A_AL2::run()
{
	TMS7000DebugHelper helper( cpu_, disass_ );
	ConsoleDebugger debugger( systemConsole_, helper, mode_ );

	cpu_.reset();
	mode_.setMode( debug_ ? MODE_STOP : MODE_RUN );
	systemConsole_.setKbReload( 0x1000 );

	uint lastpc = 0, pc = 0, breakPoint = 0xFFFF;

	while ( mode_.getMode() != MODE_EXIT )
	{
		pc = cpu_.getPC();
		lastpc = pc;

		ExecMode mode = mode_.getMode();

		if ( mode == MODE_STOP )
		{

			debugger.display();

				pc = cpu_.getPC();

			int c = toupper( systemConsole_.getch() );

			if ( !c )
				continue;

			debugger.setBreakOn( true );
			debugger.doCommand( c );

			breakPoint = debugger.getBreakPoint();
		}
		else
		{
			lastpc = cpu_.getPC();
			cpu_.sim();

			if ( mode == MODE_STOP && !debugger.isBreakOn() )
			{
				if ( helper.isBreak( lastpc ) )
				{
					mode_.setMode( mode = MODE_RUN );
				}
			}

			if ( cpu_.getPC() == breakPoint )
			{
				mode_.setMode( mode = MODE_STOP );
				debugger.setBreakPoint( 0xFFFF );
			}
			else if ( mode == MODE_RET )
			{
				if ( helper.isRet( lastpc ) )
				{
					if ( helper.isBreakRet( lastpc, debugger.getRetSP() ) )
					{
						mode_.setMode( mode = MODE_STOP );
					}
				}
				else if ( helper.isSetSP( lastpc ) )
				{
					mode_.setMode( mode = MODE_STOP );
				}
			}

			if ( mode == MODE_STOP )
			{
				debugger.displayLast( lastpc );
			}
		}
	}

	if ( mode_.getMode() != MODE_EXIT )
	{
		systemConsole_.printf( "\nSTOP: pc=%04X - lastpc=%04X", pc, lastpc );
		systemConsole_.getch();
	}

	systemConsole_.printf( "\n" );

	if ( data_.getOption( 'M' ) == 'T' )
		printf( "\n" );
}

void CTS256A_AL2::stop()
{
	mode_.setMode( MODE_STOP );
}


void CTS256A_AL2::exit()
{
	mode_.setMode( MODE_EXIT );
}


void CTS256A_AL2::reset()
{
	cpu_.reset();
}


void CTS256A_AL2::wakeup()
{
}


void CTS256A_AL2::step()
{
}


void CTS256A_AL2::callstep()
{
}

// CTS256A_AL2 TMS7000 ROM contents (0xF000..0xFFFF)
const uchar CTS256A_AL2_ROM[0x1000] = 
{
	0x52,0x3A,0x0D,0x88,0x20,0x00,0x2D,0xA2,0xAA,0x00,0xA2,0x0A,0x10,0x91,0x04,0x53,
	0x07,0x5D,0x00,0xE2,0x39,0x73,0x7F,0x0A,0x80,0x04,0x23,0x08,0x2D,0x00,0xE2,0x07,
	0x8A,0x10,0x00,0x82,0x11,0xE0,0x03,0xA2,0xCB,0x11,0xA2,0x15,0x11,0xAA,0xF0,0x3E,
	0xB8,0xAA,0xF0,0x46,0xC9,0x92,0x15,0x82,0x14,0xA4,0x01,0x10,0xE0,0x1A,0xFF,0x40,
	0x43,0x40,0x43,0x40,0x40,0x40,0xFF,0x20,0x57,0x07,0xC2,0x0F,0x81,0x03,0x74,0x80,
	0x0A,0x88,0x02,0x00,0x2F,0xA4,0x30,0x00,0x91,0x04,0x53,0x10,0x5D,0x00,0xE2,0x59,
	0x88,0x30,0x00,0x03,0x98,0x03,0x29,0xD8,0x02,0xD2,0x02,0x42,0x02,0x22,0x72,0xFF,
	0x23,0x78,0x02,0x02,0xD5,0x17,0xD3,0x02,0x7D,0xF0,0x02,0xE2,0x1E,0xD3,0x17,0x7D,
	0x10,0x17,0xE2,0x17,0x22,0x5A,0x9B,0x03,0xB5,0x9A,0x03,0x2D,0x5A,0xE6,0x0C,0xB7,
	0x9B,0x03,0xB5,0x9A,0x03,0x2D,0xA5,0xE6,0x02,0xE0,0xDB,0x42,0x02,0x26,0xD5,0x27,
	0x7A,0x01,0x02,0x98,0x03,0x07,0x98,0x07,0x2B,0x7A,0x01,0x02,0x42,0x02,0x24,0x72,
	0xFF,0x25,0xD9,0x02,0x72,0xDF,0x32,0xE0,0x1D,0x88,0x00,0x51,0x29,0x88,0x00,0x65,
	0x25,0x88,0x00,0x50,0x23,0x88,0x00,0x66,0x2B,0x88,0x00,0x80,0x27,0x98,0x29,0x03,
	0x98,0x2B,0x07,0x72,0x01,0x32,0x88,0x00,0x00,0x13,0xC5,0x78,0x10,0x12,0x7D,0xF0,
	0x12,0xE2,0x17,0x9A,0x13,0xAD,0xF5,0x26,0xE6,0xF0,0xC3,0x5D,0x05,0xE2,0x04,0xD3,
	0x13,0xE0,0xF0,0xD3,0x13,0x98,0x13,0x31,0x9C,0x31,0x72,0x00,0x30,0x8E,0xF1,0x43,
	0x8E,0xF1,0xAC,0xE0,0x0B,0x76,0x01,0x0B,0x07,0x73,0xEF,0x0B,0x77,0x10,0x0B,0xFC,
	0x4D,0x03,0x05,0xE6,0x07,0x4D,0x02,0x04,0xE6,0x02,0xE0,0xE9,0x7D,0x00,0x38,0xE6,
	0x05,0x7D,0x00,0x39,0xE2,0xF6,0x77,0x08,0x0B,0x09,0x7D,0x01,0x32,0xE2,0x11,0x76,
	0x08,0x0B,0xFC,0x8E,0xF3,0xE7,0x4D,0x07,0x09,0xE2,0xD5,0xA4,0x01,0x00,0xE0,0xD0,
	0x8C,0xF1,0xF0,0x73,0x00,0x0B,0xD5,0x37,0xD5,0x38,0xD5,0x39,0xA4,0x01,0x06,0x98,
	0x29,0x03,0x98,0x2B,0x07,0x22,0x20,0x9B,0x03,0x8E,0xF7,0x2B,0x98,0x03,0x05,0xD8,
	0x03,0xD8,0x07,0xD8,0x06,0xD2,0x03,0x4A,0x03,0x07,0x4B,0x02,0x06,0x98,0x07,0x34,
	0xDB,0x34,0xB0,0xDD,0x06,0xDD,0x07,0x98,0x07,0x1F,0xDD,0x06,0xDD,0x07,0xB0,0xDD,
	0x06,0xDD,0x07,0x98,0x07,0x21,0xD9,0x06,0xD9,0x07,0xD9,0x03,0x98,0x07,0x09,0x98,
	0x03,0x19,0x98,0x27,0x36,0x4A,0x2B,0x36,0x4B,0x2A,0x35,0x91,0x04,0x53,0x80,0x5D,
	0x00,0xE2,0x03,0x74,0x01,0x0B,0x05,0x0A,0x4F,0x2D,0x4B,0x0D,0x73,0xF9,0x0A,0xC5,
	0xAA,0xF1,0xA8,0x8E,0xF1,0xE2,0xC3,0x5D,0x04,0xE6,0xF5,0x0A,0xA6,0x02,0x11,0x01,
	0x0B,0xB8,0x76,0x80,0x0A,0x0B,0xA3,0xFE,0x10,0xA7,0x02,0x11,0xFC,0x80,0x16,0xE0,
	0x05,0xA3,0xEF,0x00,0x9A,0x2F,0x8E,0xF1,0xE2,0xB9,0x76,0x20,0x0B,0x03,0x8E,0xF2,
	0x8C,0x0B,0xC8,0xD8,0x0A,0xD8,0x0C,0xD8,0x0D,0x73,0xF9,0x0A,0x2D,0x1B,0xE6,0x14,
	0xA3,0xFE,0x00,0x8E,0xF1,0x43,0x52,0x3A,0x0D,0x98,0x2D,0x1B,0x9B,0x1B,0x8E,0xF2,
	0x8C,0x8C,0xF1,0x05,0x2D,0x12,0xE6,0x19,0x76,0x01,0x0B,0x12,0x4A,0x19,0x03,0x4B,
	0x18,0x02,0x4A,0x03,0x34,0x4B,0x02,0x33,0x98,0x19,0x03,0x72,0x01,0x39,0x8C,0xF2,
	0x84,0x2D,0x08,0xE6,0x23,0x4D,0x02,0x04,0xE6,0x05,0x4D,0x03,0x05,0xE2,0x55,0xD8,
	0x03,0xD8,0x02,0x98,0x05,0x03,0x8E,0xF7,0x3B,0x98,0x03,0x05,0xD9,0x02,0xD9,0x03,
	0xD3,0x34,0xE7,0x40,0xD3,0x33,0xE0,0x3C,0x2D,0x27,0xE2,0x26,0x2D,0x7B,0xE4,0x0C,
	0x2D,0x30,0xE1,0x08,0x2D,0x3A,0xE1,0x1A,0x2D,0x41,0xE5,0x16,0x76,0x01,0x0B,0x07,
	0x2D,0x0D,0xE6,0x06,0x74,0x10,0x0B,0x98,0x03,0x19,0x24,0x80,0xD3,0x39,0xE7,0x02,
	0xD3,0x38,0x8E,0xF2,0x98,0x77,0x20,0x0B,0x0B,0x22,0x8D,0xD3,0x39,0xE7,0x02,0xD3,
	0x38,0x8E,0xF2,0x98,0xD9,0x0D,0xD9,0x0C,0xD9,0x0A,0xC9,0x0A,0x77,0x80,0x0A,0x04,
	0xA4,0x10,0x00,0x0A,0xA4,0x01,0x10,0x0A,0x76,0x02,0x0A,0x2E,0x76,0x04,0x0A,0x07,
	0x98,0x05,0x0D,0xDB,0x34,0xE0,0x05,0x98,0x09,0x0D,0xDB,0x36,0x9B,0x0D,0x8A,0x00,
	0x0D,0x78,0x01,0x0D,0x79,0x00,0x0C,0x8E,0xF3,0x11,0x76,0x04,0x0A,0x07,0x98,0x0D,
	0x05,0x8E,0xF3,0x31,0x0A,0x98,0x0D,0x09,0xE0,0xF7,0x76,0x04,0x0A,0x0B,0x98,0x03,
	0x0D,0x76,0x02,0x0B,0x0D,0xD3,0x37,0xE0,0x09,0x98,0x07,0x0D,0xD3,0x36,0xE7,0x02,
	0xD3,0x35,0x9A,0x0D,0x76,0x04,0x0A,0x0B,0x27,0x80,0x05,0x74,0x01,0x0A,0xE0,0x03,
	0x73,0xFE,0x0A,0xB8,0x8A,0x00,0x0D,0x78,0x01,0x0D,0x79,0x00,0x0C,0x8E,0xF3,0x11,
	0x76,0x04,0x0A,0x05,0x98,0x0D,0x03,0xB9,0x0A,0x98,0x0D,0x07,0x8E,0xF3,0x35,0xB9,
	0x0A,0x76,0x04,0x0A,0x0E,0x4D,0x2B,0x0D,0xE6,0x08,0x4D,0x2A,0x0C,0xE6,0x03,0x98,
	0x29,0x0D,0x0A,0x4D,0x27,0x0D,0xE6,0x08,0x4D,0x26,0x0C,0xE6,0x03,0x98,0x2B,0x0D,
	0x0A,0x77,0x04,0x0A,0x12,0x7D,0x01,0x35,0xE2,0x09,0x4D,0x32,0x36,0xE4,0x04,0x74,
	0x08,0x0B,0x0A,0x73,0xF7,0x0B,0x0A,0x7D,0x00,0x33,0xE4,0x0A,0x7D,0x01,0x34,0xE4,
	0x05,0x74,0x20,0x0B,0xE0,0x1B,0x4D,0x1E,0x33,0xE1,0x07,0xE4,0x1B,0x4D,0x1F,0x34,
	0xE5,0x16,0x4D,0x20,0x33,0xE4,0x1D,0xE1,0x05,0x4D,0x21,0x34,0xE4,0x16,0x74,0x04,
	0x0B,0xA3,0xFE,0x06,0xA2,0x13,0x17,0x0A,0x73,0xDB,0x0B,0x8E,0xF2,0x8C,0xA4,0x01,
	0x06,0xA2,0x11,0x17,0x0A,0xA3,0xFE,0x00,0xB8,0xC8,0xD8,0x0A,0xD8,0x0C,0xD8,0x0D,
	0x74,0x06,0x0A,0x8E,0xF2,0x98,0x98,0x2D,0x1B,0x48,0x00,0x1B,0x9B,0x1B,0xD9,0x0D,
	0xD9,0x0C,0xD9,0x0A,0xC9,0xB9,0x4D,0x07,0x09,0xE2,0x03,0xA4,0x01,0x00,0x0B,0x2D,
	0x30,0xE5,0x02,0xE0,0x2A,0x2D,0x3A,0xE5,0x08,0x88,0xFF,0x8E,0x15,0x73,0xDF,0x0A,
	0x0A,0x2D,0x41,0xE5,0x02,0xE0,0x18,0x2D,0x5B,0xE5,0x04,0x74,0x20,0x0A,0x0A,0x2D,
	0x61,0xE5,0x02,0xE0,0x0A,0x2D,0x7B,0xE5,0x06,0x2A,0x20,0x74,0x20,0x0A,0x0A,0x73,
	0xDF,0x0A,0x88,0xF7,0x8C,0x15,0x0A,0x7D,0x00,0x30,0xE2,0x02,0x9C,0x31,0x98,0x03,
	0x11,0x8E,0xF7,0x4B,0x8E,0xF7,0x0F,0x77,0x01,0x0A,0x05,0x74,0x80,0x0B,0xE0,0x03,
	0x73,0x7F,0x0B,0x8E,0xF3,0xAF,0x77,0x20,0x0A,0x11,0xC5,0x2A,0x41,0x2C,0x02,0x58,
	0x02,0xAA,0xFF,0xBC,0xD0,0x14,0xAA,0xFF,0xBD,0xD0,0x15,0x52,0x01,0x8E,0xF4,0x88,
	0x8E,0xF4,0xC2,0x76,0x10,0x0A,0x40,0x98,0x11,0x1D,0x73,0xBF,0x0A,0x8E,0xF5,0x64,
	0x76,0x10,0x0A,0x33,0x8E,0xF4,0x7E,0x74,0x40,0x0A,0x8E,0xF5,0x64,0x76,0x10,0x0A,
	0x39,0x48,0x37,0x34,0x79,0x00,0x33,0xD5,0x37,0x73,0xFD,0x0B,0x52,0x02,0x8E,0xF4,
	0x88,0x8E,0xF4,0x9E,0x98,0x0F,0x03,0x98,0x03,0x11,0x8E,0xF7,0x4B,0x77,0x80,0x0B,
	0x93,0xDB,0x39,0x8E,0xF3,0x47,0x0A,0xD3,0x15,0xE7,0x02,0xD3,0x14,0x52,0x02,0x8E,
	0xF4,0x88,0x72,0x01,0x37,0x73,0xFD,0x0B,0xE0,0xA6,0x52,0x03,0xE0,0xF1,0x9A,0x15,
	0x27,0x40,0x01,0x0A,0xDB,0x15,0xE0,0xF6,0xD5,0x17,0x9A,0x15,0x27,0x40,0x07,0xD3,
	0x17,0x3D,0x17,0xE6,0x01,0x0A,0xD3,0x15,0xE7,0x02,0xD3,0x14,0xE0,0xEC,0xD5,0x17,
	0x9A,0x15,0x2D,0xFF,0xE2,0x1B,0x27,0x80,0x02,0xD3,0x17,0x23,0x3F,0x73,0xFD,0x0A,
	0x74,0x04,0x0A,0x8E,0xF2,0x98,0xD3,0x15,0xE7,0x02,0xD3,0x14,0x7D,0x01,0x17,0xE6,
	0xDD,0x0A,0x98,0x03,0x13,0x73,0xF7,0x0A,0x77,0x20,0x0A,0x08,0x9A,0x15,0x2D,0xFF,
	0xE6,0x0B,0xE0,0x2E,0x2D,0xFF,0xE2,0x2A,0x8E,0xF7,0x3B,0xD2,0x37,0x8E,0xF7,0x0F,
	0x2D,0x61,0xE1,0x02,0x2A,0x20,0x2A,0x20,0xC0,0x9A,0x15,0x27,0x80,0x03,0x74,0x08,
	0x0A,0x23,0x3F,0x3D,0x00,0xE2,0x07,0x74,0x10,0x0A,0x98,0x13,0x03,0x0A,0x77,0x08,
	0x0A,0x0A,0x98,0x03,0x0F,0x73,0xEF,0x0A,0x74,0x02,0x0B,0x0A,0xD3,0x15,0xE7,0xCD,
	0xD3,0x14,0xE0,0xC9,0x32,0x16,0x5D,0x3A,0xE4,0x0A,0x5D,0x21,0xE1,0x06,0x5A,0x21,
	0xAA,0xF5,0x26,0x0A,0xB5,0x0A,0x80,0x48,0x28,0x58,0x85,0x08,0x68,0x08,0x84,0x78,
	0x08,0x58,0x48,0x58,0x82,0x08,0x08,0x58,0x38,0x18,0x82,0x48,0x48,0x28,0x84,0x78,
	0x8C,0xF6,0x9E,0x8C,0xF6,0xB4,0x8C,0xF6,0x01,0x8C,0xF6,0xD7,0x8C,0xF6,0xE2,0x8C,
	0xF6,0xBF,0x8C,0xF6,0xC7,0x8C,0xF5,0xEA,0x8C,0xF5,0xB9,0x8C,0xF6,0xCF,0x8C,0xF5,
	0xB0,0x8C,0xF5,0xCC,0x76,0x40,0x0A,0x0A,0x52,0x40,0xD3,0x15,0xE7,0x02,0xD3,0x14,
	0xE0,0x04,0x52,0x80,0xDB,0x15,0x9A,0x15,0x67,0x12,0x76,0x40,0x0A,0x04,0xDB,0x15,
	0xE0,0x06,0xD3,0x15,0xE7,0x02,0xD3,0x14,0x73,0xEF,0x0A,0x0A,0x8E,0xF7,0x5B,0x9A,
	0x15,0x2D,0x15,0xE1,0x0F,0x4D,0x00,0x16,0xE2,0xCA,0x74,0x10,0x0A,0x98,0x13,0x03,
	0x98,0x1D,0x11,0x0A,0x2D,0x07,0xE2,0xED,0xC0,0x5A,0x09,0x5C,0x03,0xAC,0xF5,0x40,
	0x8E,0xF5,0x14,0x2D,0x00,0xE2,0x47,0xE0,0xE1,0x8E,0xF5,0x14,0x26,0x08,0x02,0xE0,
	0xD9,0x8E,0xF7,0x5B,0x8E,0xF5,0x14,0x26,0x08,0xF7,0xE0,0x2F,0x8E,0xF5,0x14,0x26,
	0x80,0x02,0xE0,0xC6,0x8E,0xF7,0x5B,0x8E,0xF5,0x14,0x26,0x80,0x02,0xE0,0xBB,0x8E,
	0xF7,0x5B,0x8E,0xF5,0x14,0x26,0x80,0xF7,0xE0,0x11,0x8E,0xF5,0x14,0x26,0x08,0x02,
	0xE0,0x09,0x8E,0xF7,0x5B,0x8E,0xF5,0x14,0x26,0x08,0xF7,0x8E,0xF7,0x7F,0x8C,0xF5,
	0x64,0x8E,0xF5,0x14,0x26,0x01,0x50,0x7D,0x29,0x16,0xE2,0x30,0x7D,0x2D,0x16,0xE2,
	0x07,0x7D,0x2F,0x16,0xE2,0x1D,0xE0,0x71,0x8E,0xF7,0x0F,0x2D,0x45,0xE2,0x02,0xE0,
	0x68,0x8E,0xF7,0x0F,0x2D,0x4E,0xE2,0x02,0xE0,0x5F,0x8E,0xF7,0x0F,0x2D,0x54,0xE2,
	0x1D,0xE0,0x56,0x8E,0xF7,0x0F,0x2D,0x52,0xE2,0x14,0xE0,0x4D,0x8E,0xF7,0x0F,0x2D,
	0x4E,0xE2,0x02,0xE0,0x44,0x8E,0xF7,0x0F,0x2D,0x47,0xE2,0x02,0xE0,0x3B,0x8E,0xF6,
	0x8C,0x76,0x20,0x0A,0x34,0xE0,0x65,0x8E,0xF7,0x0F,0x2D,0x52,0xE6,0x0C,0x8E,0xF7,
	0x0F,0x2D,0x53,0xE2,0xE9,0x8E,0xF7,0x3B,0xE0,0xE4,0x2D,0x53,0xE2,0xE0,0x2D,0x44,
	0xE2,0xDC,0x2D,0x4C,0xE2,0x0C,0x8E,0xF7,0x3B,0x8E,0xF6,0x8C,0x77,0x20,0x0A,0x3C,
	0xE0,0x07,0x8E,0xF7,0x0F,0x2D,0x59,0xE2,0xC5,0x8C,0xF5,0x9A,0x8E,0xF7,0x0F,0xD8,
	0x15,0xD8,0x14,0x8E,0xF3,0xAF,0xD9,0x14,0xD9,0x15,0x8E,0xF7,0x3B,0x0A,0x8E,0xF5,
	0x14,0x26,0x80,0x02,0xE0,0xE3,0x8E,0xF7,0x5B,0x8E,0xF5,0x14,0x26,0x80,0xF7,0x8E,
	0xF7,0x7F,0xE0,0x08,0x8E,0xF5,0x14,0x26,0x40,0x02,0xE0,0xCD,0x8C,0xF5,0x64,0x8E,
	0xF5,0x14,0x26,0x08,0xF7,0xE0,0xC2,0x8E,0xF5,0x14,0x26,0x04,0xEF,0xE0,0xBA,0x8E,
	0xF5,0x14,0x26,0x02,0xE7,0xE0,0xB2,0x8E,0xF5,0x14,0x26,0x20,0xDF,0x8E,0xF7,0x5B,
	0xE0,0x0E,0x8E,0xF5,0x14,0x26,0x10,0xD4,0x8E,0xF7,0x5B,0x7D,0x34,0x16,0xE2,0x0C,
	0x7D,0x23,0x16,0xE2,0x07,0x7D,0x33,0x16,0xE2,0x02,0xE0,0x8D,0x98,0x11,0x03,0x8E,
	0xF7,0x2B,0x8E,0xF7,0x2B,0x8E,0xF7,0x0F,0x2D,0x48,0xE2,0xB0,0x8C,0xF5,0x9A,0x74,
	0x02,0x0A,0x73,0xFB,0x0A,0x8E,0xF2,0x98,0x23,0x7F,0x0A,0xD3,0x11,0xE7,0x02,0xD3,
	0x10,0x98,0x11,0x0D,0x8E,0xF3,0x15,0x98,0x0D,0x11,0x0A,0xD3,0x03,0xE7,0x02,0xD3,
	0x02,0x98,0x03,0x0D,0x8E,0xF3,0x15,0x98,0x0D,0x03,0x0A,0xDB,0x03,0x4D,0x23,0x03,
	0xE6,0x08,0x4D,0x22,0x02,0xE6,0x03,0x98,0x25,0x03,0x0A,0xDB,0x11,0x4D,0x23,0x11,
	0xE6,0x08,0x4D,0x22,0x10,0xE6,0x03,0x98,0x25,0x11,0x0A,0x77,0x40,0x0A,0x07,0xD8,
	0x03,0xD8,0x02,0x98,0x11,0x03,0x8E,0xF7,0x0F,0x77,0x40,0x0A,0x07,0x8E,0xF7,0x4B,
	0xD9,0x02,0xD9,0x03,0x2D,0x61,0xE1,0x02,0x2A,0x20,0x2A,0x20,0xD0,0x16,0x0A,0x76,
	0x40,0x0A,0x04,0x8E,0xF7,0x3B,0x0A,0x8E,0xF7,0x1B,0x0A,0xFF,0xCD,0xC0,0x0A,0x47,
	0xB3,0xEB,0x09,0x10,0x0A,0x25,0x47,0xB3,0xEB,0x09,0x47,0xB3,0xEB,0x47,0xB3,0xF7,
	0xC7,0xFF,0xCC,0xC3,0xDB,0xC3,0xC0,0xC1,0xCE,0x44,0x84,0xC1,0x44,0x84,0xDF,0x44,
	0x84,0xDA,0xC4,0xC5,0x49,0x34,0x37,0x37,0x07,0x0B,0x0D,0x80,0xC4,0x61,0x18,0x2D,
	0x33,0x2B,0x80,0xC3,0x78,0x0F,0x10,0x1C,0x33,0x80,0xFF,0xC2,0x13,0xFF,0x13,0xD4,
	0x63,0x28,0xA5,0x54,0x02,0xA9,0xFF,0x13,0xCF,0x13,0x72,0xA5,0x13,0xFB,0x0E,0xF3,
	0x09,0x54,0xB7,0x13,0xF2,0x2F,0x4F,0xA7,0x13,0xFF,0x0E,0x32,0xCF,0xF2,0x09,0xEF,
	0x13,0x10,0x6E,0xB9,0x47,0x0B,0x93,0x67,0x21,0x29,0xAE,0x4F,0x01,0x24,0x07,0x07,
	0x8B,0xFF,0x37,0x21,0xCF,0xF7,0x57,0x97,0x13,0x10,0xFF,0x0E,0x0F,0x13,0xD4,0x13,
	0xFF,0x0E,0x09,0xCF,0xFF,0x0E,0x0F,0x09,0xD4,0x09,0x10,0x6C,0x2C,0xB9,0x4F,0x2D,
	0x93,0x13,0xEC,0x09,0x4F,0xAD,0x09,0x10,0xE7,0x25,0x4C,0x01,0x8A,0xFF,0x0E,0x0B,
	0xD4,0xFF,0x0E,0x0F,0x10,0x09,0xDA,0x13,0x72,0xB2,0x4F,0xA7,0x72,0xB2,0x5A,0xA7,
	0x13,0x10,0xF2,0x13,0xFB,0xF2,0x13,0xF3,0xF2,0xFB,0x69,0xB2,0x47,0xAF,0xE9,0xD4,
	0xF9,0xD4,0xF5,0xD7,0x09,0x10,0xEC,0x13,0xFE,0x09,0x10,0x6C,0xB3,0x13,0x7E,0xAB,
	0x6C,0xAB,0x57,0x02,0xA9,0xFF,0x2C,0x0E,0xD7,0x13,0x10,0x62,0x2C,0xA5,0x54,0x01,
	0x3F,0xBE,0x62,0x2C,0xA5,0x4F,0x01,0x3F,0xBE,0x6E,0xA7,0x0F,0x54,0x0B,0x01,0x8A,
	0xFF,0xDA,0x13,0xFF,0x13,0x41,0x3F,0x93,0x2D,0x21,0x39,0xE5,0x7F,0x93,0x13,0xE5,
	0x0E,0x09,0x41,0x3F,0x93,0x13,0x65,0x25,0xAE,0x13,0x7F,0x0C,0x8B,0x13,0x6F,0x34,
	0xA8,0x13,0x41,0x3F,0x35,0x9D,0x13,0x75,0xB3,0x09,0x41,0x3F,0x0C,0xAB,0x75,0x29,
	0xAC,0x41,0x3F,0x0C,0x0C,0xAD,0xFF,0x22,0xFF,0xFF,0x13,0x41,0x9C,0xFF,0x33,0x41,
	0x9C,0xF4,0x42,0x8D,0x13,0xFF,0x0E,0x41,0x9C,0xFF,0x41,0xBF,0x13,0xFF,0x13,0x77,
	0x37,0x93,0x13,0xE8,0x0E,0x42,0xAA,0x0E,0x25,0xE8,0x42,0xAA,0xE8,0x42,0xB2,0x33,
	0xE9,0x09,0x77,0x37,0x86,0xE9,0x09,0xE5,0xE9,0x2F,0xE5,0xE9,0x25,0x2E,0xE5,0xFF,
	0x0F,0x77,0xB7,0x23,0xFF,0xFF,0xEB,0x09,0x42,0xAA,0xEB,0x42,0xA9,0x6F,0xAD,0x0B,
	0x42,0x08,0x0F,0x90,0xE3,0x0F,0x42,0x2A,0x37,0xB7,0xFF,0x13,0x42,0xA9,0xFF,0x33,
	0x42,0xA9,0xFF,0x12,0x42,0x88,0xFF,0x42,0xAA,0x13,0xFF,0x13,0x41,0x21,0x93,0xFF,
	0x24,0xFF,0x09,0x10,0x65,0xA4,0x13,0x41,0x21,0x0C,0x01,0x95,0x0A,0x25,0xFF,0x13,
	0x41,0x95,0x09,0x11,0x25,0xFF,0x13,0x42,0x8D,0x13,0xE5,0x0E,0x09,0x41,0x21,0x8C,
	0x13,0xEF,0x13,0x41,0x21,0x9F,0x13,0x6F,0x25,0xB3,0x41,0x21,0x0F,0xAB,0x13,0x6F,
	0x29,0x2E,0xA7,0x41,0x21,0x1F,0x0C,0xAC,0x13,0x6F,0xB7,0x41,0x21,0xA0,0x09,0xF5,
	0x10,0x21,0x41,0x0A,0x96,0xE7,0x41,0x8A,0xEA,0x41,0x8A,0xFF,0x13,0x41,0x95,0xFF,
	0x33,0x41,0x95,0xFF,0x41,0xA1,0x13,0xFF,0x13,0xD3,0x09,0x10,0xFF,0x13,0xFF,0x07,
	0x11,0xFF,0x13,0xFF,0x11,0xFF,0x13,0xD3,0x09,0xE4,0x13,0x41,0x95,0x09,0x10,0xFF,
	0x24,0x13,0xFF,0xF6,0x25,0x32,0x47,0xA3,0x09,0x11,0xEC,0xFE,0x72,0xA9,0x09,0x7C,
	0x93,0x09,0x10,0xF2,0x09,0xF3,0xFF,0x0E,0x0B,0xD3,0x72,0xA9,0x47,0x07,0x0E,0x8C,
	0xF2,0x09,0x47,0xAF,0xF2,0xF3,0x13,0x76,0x25,0xAE,0x13,0x53,0x23,0x0C,0x8B,0x13,
	0x76,0x25,0xAE,0x53,0x23,0x07,0x07,0x8B,0x09,0x10,0xF7,0x71,0x9F,0x0D,0xF7,0xDF,
	0xF7,0x71,0x9F,0xFF,0x2F,0xD3,0x09,0x10,0x0C,0xF3,0x13,0x4C,0xAB,0x09,0x10,0xFF,
	0x33,0x13,0xFF,0x09,0x10,0x6C,0xB9,0x13,0x6D,0x93,0x09,0x10,0x6D,0x25,0x2E,0xB4,
	0x50,0x0C,0x0B,0x02,0x8D,0x66,0x35,0xAC,0x68,0x1E,0xAD,0x65,0xB2,0xFC,0xE5,0xD3,
	0x61,0x32,0xAE,0x74,0x8B,0x13,0x61,0xB2,0x0E,0xF4,0x11,0x61,0xB2,0xFC,0x61,0xA4,
	0x47,0x07,0x01,0x95,0x09,0x10,0xE1,0x13,0x53,0x8F,0xE1,0x33,0x35,0xC7,0xE1,0xD3,
	0x69,0x27,0xA8,0xD4,0xE9,0xD3,0x13,0x79,0xA5,0xC6,0xF9,0xD3,0xF5,0xD6,0xFF,0xC7,
	0x13,0xFF,0x13,0x47,0x07,0xA8,0xF5,0x2C,0x68,0x9E,0xFF,0x26,0xFF,0x6F,0x35,0xB2,
	0x68,0xBA,0xFF,0xE8,0x13,0xFF,0x13,0x41,0x0A,0x93,0x69,0xB6,0x41,0x24,0x0C,0xA3,
	0x13,0xFF,0x29,0x0E,0x41,0xA4,0xE5,0x34,0x41,0x24,0x87,0x33,0x35,0x67,0x25,0xB3,
	0x41,0x3D,0x01,0x0A,0x07,0x07,0xB7,0xE7,0x41,0xA4,0x72,0x25,0x21,0xB4,0x41,0x22,
	0x27,0x14,0x8D,0xFF,0x13,0x41,0xA2,0x13,0x22,0x09,0xFF,0x41,0xBD,0xFF,0x0F,0x41,
	0x8A,0x09,0xE8,0xE8,0xE8,0x41,0xBD,0xFF,0x41,0xBD,0x13,0xFF,0x13,0x54,0x02,0xB2,
	0x13,0x61,0xB6,0x5B,0x1A,0xA3,0x13,0x65,0x32,0xA5,0x5B,0xBC,0x13,0x6F,0x35,0xB2,
	0x60,0xB3,0x6F,0xB7,0x5B,0xA0,0x79,0xB0,0x5B,0x0C,0x02,0x89,0xFF,0x12,0xF9,0xFF,
	0x09,0xDB,0xFF,0xFF,0x13,0xEE,0x4C,0x8B,0x2E,0xFF,0x2E,0x25,0xC6,0xFF,0x13,0xC6,
	0xEE,0x24,0x46,0x8B,0x13,0x10,0xFF,0x0B,0xC6,0x13,0x10,0x65,0xA4,0x13,0x46,0x01,
	0x95,0x09,0x11,0x65,0xA4,0x13,0x53,0x01,0x95,0x26,0x32,0xE5,0x2E,0x24,0xC7,0x65,
	0xAE,0x53,0x0C,0x8B,0xE5,0x34,0x46,0x8C,0x65,0xB2,0x53,0xB3,0xFF,0x0B,0xD3,0xE5,
	0xD3,0xEE,0x0B,0x53,0x8B,0xF2,0x09,0x46,0xB3,0xFF,0x0E,0x0B,0xC6,0xFF,0x0E,0x0F,
	0x10,0x09,0xCC,0xFA,0x0B,0x46,0xAB,0xF3,0x0B,0x46,0xAB,0xFF,0x1F,0x0B,0xC6,0x0F,
	0x0E,0xFF,0x0E,0x0F,0xCC,0xFF,0x34,0x0B,0xC6,0x09,0x11,0xFF,0x0E,0x0F,0xCC,0xF2,
	0xF4,0x11,0xFF,0x2F,0x2E,0xF1,0x67,0xA8,0xC6,0x6C,0xA4,0x46,0x3E,0x01,0x95,0x67,
	0xAE,0x46,0x8B,0x67,0xAE,0x0E,0x46,0x8B,0x67,0xAE,0x0B,0x46,0x8B,0x71,0x35,0xA5,
	0x53,0x02,0xA9,0xFF,0x21,0xC6,0x2D,0xFF,0x23,0xC6,0xFF,0xCC,0x13,0xFF,0x13,0x41,
	0x0A,0x94,0xFF,0x41,0x8A,0x13,0xFF,0x13,0x42,0x2A,0x94,0x13,0xFF,0x2E,0xFF,0xFF,
	0x13,0x42,0xA9,0xFF,0x42,0xAA,0x13,0xFF,0x13,0x47,0x07,0xAD,0xEF,0x23,0x09,0x6D,
	0xB5,0xFF,0x2C,0xFF,0xFF,0x0B,0xFE,0x65,0x21,0xA4,0x6D,0x13,0x01,0x95,0x61,0x35,
	0x27,0xA8,0x6D,0x1A,0xA8,0xFF,0xED,0xE2,0xD0,0x13,0xFF,0x13,0x47,0x07,0x90,0x6F,
	0xB6,0x50,0x1F,0xA3,0xFF,0x2D,0xFF,0xFF,0xD0,0x13,0xFF,0x13,0x47,0x07,0x8B,0x25,
	0xE7,0x0F,0x4B,0x01,0x8A,0xE7,0x32,0x6C,0x01,0xA4,0xE7,0x09,0x6C,0x01,0xA4,0x67,
	0xAC,0x0B,0x6C,0x01,0x24,0xBE,0xE7,0xEC,0xEB,0x13,0x6C,0x02,0xA9,0xEB,0x33,0x6C,
	0x02,0xA9,0xEB,0x6C,0x02,0xAA,0x13,0x6F,0xB7,0x13,0x78,0xA0,0xFF,0x2E,0xFF,0x09,
	0x10,0xF5,0x4B,0x31,0x96,0x13,0xFF,0xF8,0x47,0xB4,0x4B,0x02,0x8D,0xFF,0xCB,0x13,
	0xFF,0x13,0xF5,0xE6,0x13,0x4F,0xA3,0x72,0x2F,0x35,0x27,0xA8,0x4F,0x0F,0x27,0xB5,
	0x09,0x10,0xF2,0x13,0xF3,0x09,0x10,0x72,0xB3,0x13,0x73,0xAB,0xF2,0xFA,0x13,0x6E,
	0xA5,0x6E,0x0F,0x8B,0x0F,0x6E,0xA5,0x6E,0x0F,0x8B,0x11,0xF7,0x2E,0xE0,0xF7,0xF5,
	0x13,0x76,0x25,0xB2,0x75,0x23,0xB3,0xF6,0x4F,0xA3,0xFF,0x0E,0x0B,0xF5,0xFF,0x0E,
	0x25,0x2E,0xF5,0xFF,0x0E,0x29,0x09,0xF5,0xEC,0x24,0x75,0xAD,0x75,0x27,0x28,0xB4,
	0x57,0x17,0x02,0x8D,0x75,0x27,0xA8,0x4F,0x0F,0xA8,0x0C,0x75,0xB2,0xFA,0x10,0x75,
	0xB2,0x60,0xB3,0x13,0xF5,0xE0,0x10,0xF5,0x33,0x09,0xE0,0x75,0xB3,0x4F,0xB7,0x75,
	0x2C,0xA4,0x5E,0x01,0x95,0x0E,0xF5,0x0E,0x2C,0xCF,0x75,0xB0,0x5F,0x02,0x89,0xF5,
	0xE0,0xF9,0xC5,0x69,0x2E,0xA7,0x75,0x0C,0xAC,0xE9,0xC5,0x6F,0xB2,0xFA,0x6F,0xAB,
	0x13,0x5E,0x02,0xA9,0x6F,0xAB,0x33,0x5E,0x02,0xA9,0x6F,0xAB,0x5E,0x02,0xAA,0x6F,
	0xA4,0x13,0x5E,0x01,0x95,0xEF,0x24,0xDE,0xEF,0xDF,0xFF,0x25,0xF5,0xFF,0x13,0xF5,
	0x61,0xB2,0xFA,0xE1,0xF5,0x13,0x6E,0x2C,0xB9,0x75,0x0B,0x2D,0x93,0x13,0x6E,0x23,
	0xA5,0x6E,0x0F,0x0B,0xB7,0x6E,0x07,0xB4,0x75,0x0B,0x02,0x8D,0x23,0xFF,0x2E,0xCF,
	0xFF,0x2E,0x27,0xD7,0x13,0x11,0xFF,0x2E,0xCF,0x29,0xEE,0x4F,0x8B,0x09,0x10,0xEE,
	0x13,0x4F,0x8B,0xFF,0x33,0x34,0x13,0xF5,0xE6,0x0E,0x57,0xA8,0x74,0x28,0x25,0xB2,
	0x4F,0x36,0xB3,0x73,0xB3,0x13,0x57,0x17,0x37,0xB7,0x09,0x11,0xED,0x4F,0x90,0xFF,
	0xD8,0x73,0x39,0x23,0xA8,0x77,0x37,0x06,0x01,0xAA,0x13,0xFF,0x13,0x42,0x09,0x93,
	0xE8,0xE8,0x65,0x2F,0xB0,0x42,0x09,0x13,0x02,0x89,0x6F,0xB7,0x42,0x09,0xA0,0x75,
	0xB4,0x13,0x42,0x09,0x1E,0x02,0x8D,0xFF,0x30,0xFF,0xFF,0x42,0x89,0x13,0xFF,0x13,
	0x42,0x2A,0x31,0x9F,0x75,0x21,0xB2,0x42,0x08,0x30,0x98,0x75,0xA5,0x13,0x42,0x2A,
	0x31,0x9F,0xF5,0x42,0x08,0xB0,0xFF,0x42,0x88,0x13,0xFF,0x13,0xFB,0x13,0xE5,0x0E,
	0x09,0x4E,0x93,0xE8,0xCE,0xFF,0x32,0xFF,0x11,0xFF,0xE7,0xFF,0xCE,0x13,0xFF,0x13,
	0x47,0x07,0x37,0xB7,0xE8,0xE5,0x09,0x69,0x2F,0xAE,0x66,0x0F,0x8B,0x6F,0x2D,0xA5,
	0x77,0x0F,0x90,0x09,0x75,0xB2,0x09,0x66,0xB3,0x75,0xB2,0x09,0x65,0xB3,0x09,0xF5,
	0x09,0x66,0x96,0x09,0x73,0xB5,0x09,0x65,0x96,0x09,0x65,0xA4,0x13,0x6B,0x01,0x95,
	0x09,0xFF,0x09,0xEB,0x61,0x29,0xA4,0x77,0x37,0x07,0x07,0x01,0x95,0x0E,0x69,0x2F,
	0xAE,0x65,0x0F,0x8B,0xFF,0x33,0xFF,0x0A,0xFF,0x13,0xEB,0x09,0x10,0x0A,0x25,0xFF,
	0x13,0xEB,0x09,0x11,0x14,0xFF,0x13,0xEB,0x09,0x11,0x09,0xFF,0x13,0xF7,0x35,0xFF,
	0x13,0xF7,0x13,0x10,0x09,0xFF,0x13,0xEB,0x13,0x63,0xA8,0x77,0x37,0x02,0xA9,0xFF,
	0x23,0x0F,0xFF,0x09,0xED,0x6B,0x90,0x09,0xFF,0x2E,0x07,0xEB,0xFF,0x13,0xF7,0xFF,
	0x77,0xB7,0xFF,0x07,0x33,0x42,0x91,0x63,0xA8,0x42,0xB2,0x13,0xFF,0x13,0x42,0x0D,
	0x93,0x13,0x68,0xA5,0x13,0x09,0x52,0x93,0x13,0x68,0xA5,0x13,0x52,0x8F,0xEF,0x13,
	0x42,0x0D,0x9F,0x6F,0x24,0x21,0xB9,0x42,0x0D,0x1F,0x21,0x94,0x68,0xA1,0x0E,0x13,
	0x52,0x9A,0x13,0x68,0x29,0xB3,0x13,0x52,0x0C,0x37,0xB7,0x13,0x68,0x25,0xB9,0x52,
	0x94,0x13,0x68,0x25,0x32,0xA5,0x52,0xAF,0x13,0x68,0x25,0xB2,0x5D,0xB3,0x68,0x25,
	0xB2,0x76,0xB3,0x68,0x25,0x29,0xB2,0x52,0xAF,0x13,0x68,0x25,0xAD,0x10,0x52,0x07,
	0x90,0x68,0x25,0x33,0xA5,0x13,0x52,0x13,0xAB,0x13,0x68,0x25,0xAE,0x52,0x07,0x8B,
	0x68,0x32,0x2F,0x35,0x27,0xA8,0x13,0x5D,0x27,0x9F,0x68,0x2F,0x33,0xA5,0x52,0x35,
	0xB7,0x68,0x2F,0x35,0x27,0xA8,0x13,0x52,0xB5,0x13,0x68,0x35,0xB3,0x52,0x0F,0x37,
	0xB7,0x68,0xA5,0x13,0xD2,0xE8,0xDD,0x09,0x10,0x65,0xA4,0x13,0x42,0x0D,0x0C,0x01,
	0x95,0x33,0xE9,0x09,0x2E,0x42,0xB2,0xE9,0x2F,0xE5,0xE9,0x21,0xE5,0x69,0x25,0xAE,
	0x65,0x0F,0x8B,0x75,0xB2,0x09,0x42,0x32,0xB3,0xF5,0x21,0x42,0x32,0x96,0x13,0x77,
	0xAF,0x42,0x0D,0x9F,0xFF,0x34,0xFF,0xFF,0x33,0x42,0x91,0xFF,0x42,0x8D,0x13,0xFF,
	0x13,0x71,0x9F,0xEE,0x29,0x59,0x16,0x8B,0x13,0xEE,0x4F,0x8B,0x13,0x70,0x2F,0xAE,
	0x4F,0x02,0x09,0x18,0x8B,0x0D,0xF2,0x09,0x56,0xB3,0xF2,0x09,0x71,0x16,0xB3,0xF2,
	0x11,0xF3,0xFF,0x0E,0x13,0xCF,0xFF,0x0E,0x0E,0xCF,0xF9,0xC6,0x13,0x27,0xFF,0x09,
	0xFF,0x27,0xFF,0x0B,0xFF,0x27,0xFF,0x09,0xEE,0x0D,0xFF,0xDF,0xFF,0x71,0x96,0x13,
	0xFF,0x13,0x63,0x93,0x69,0x25,0xB7,0x63,0x31,0x9F,0xFF,0xE3,0x13,0xFF,0x13,0x41,
	0x21,0x0F,0x01,0x3F,0x3E,0x31,0x96,0x13,0x65,0x32,0xA5,0x6E,0xB4,0x13,0x61,0xB3,
	0x13,0x6E,0x0F,0xAB,0xE1,0x33,0x6E,0x98,0xE1,0x34,0x6E,0x17,0x97,0x61,0xAE,0x6E,
	0x18,0x8B,0x68,0x25,0x32,0xA5,0x70,0xAF,0x68,0x21,0xB4,0x70,0x18,0x02,0x8D,0x68,
	0x2F,0xAC,0x79,0x35,0xAD,0x68,0xAF,0x79,0x9F,0xEF,0x2D,0x6E,0x8F,0xE8,0xF0,0x61,
	0xB2,0x6E,0xBA,0x6F,0xB2,0x0E,0x6E,0xB3,0xF2,0xCE,0xFF,0xEE,0x13,0xFF,0x13,0x47,
	0x02,0x29,0xB7,0x13,0xFF,0xEB,0xFF,0x42,0x29,0xB7,0x6F,0x35,0xB2,0x59,0xBA,0x13,
	0xFF,0x13,0x6E,0x86,0x6F,0x35,0x2E,0xA7,0x59,0x0F,0xAC,0x13,0x6F,0xB5,0x59,0x9F,
	0x65,0x21,0xB2,0x10,0x59,0xBC,0x13,0x65,0xB3,0x59,0x07,0x37,0xB7,0x13,0xFF,0xD9,
	0x09,0x11,0xFF,0x13,0xD3,0x09,0x11,0xFF,0x29,0xD3,0x13,0x10,0xFF,0x13,0xC6,0x13,
	0x10,0xFF,0x09,0xC6,0x13,0x10,0xFF,0x0E,0x0F,0x10,0x09,0xCC,0x13,0x10,0xFF,0x0E,
	0x09,0xC6,0xFF,0xCC,0x13,0xFF,0x13,0x6B,0x93,0xFF,0x3A,0xFF,0xFF,0xEB,0xD0,0x6B,
	0x3C,0xB5,0xD1,0x6E,0x0F,0x0F,0x8B,0xD2,0x42,0x0D,0x9F,0xD3,0x5D,0x0E,0x93,0xD4,
	0x68,0xBA,0xD5,0x68,0x06,0xA3,0xD6,0x77,0x37,0x0C,0x02,0x29,0xB7,0xD7,0x77,0x37,
	0x07,0x23,0x0C,0x8B,0xD8,0x54,0x02,0x8D,0xD9,0x78,0x06,0x8B,0xF7,0x8C,0xF7,0xCC,
	0xF8,0x82,0xF8,0xCC,0xF9,0x19,0xF9,0x76,0xFA,0x30,0xFA,0x44,0xFA,0x8A,0xFA,0xB4,
	0xFB,0x4C,0xFB,0x55,0xFB,0x66,0xFB,0x87,0xFB,0x99,0xFB,0xDF,0xFC,0xE1,0xFD,0x0D,
	0xFD,0x29,0xFD,0x3D,0xFD,0xC2,0xFE,0x8E,0xFE,0xCF,0xFE,0xDC,0xFF,0x2C,0xFF,0x3A,
	0xFF,0x84,0xFF,0x8E,0xFF,0xFF,0xF1,0xBC,0xF1,0xC1,0xFF,0xFF,0xF3,0x85,0xF0,0x00,
};