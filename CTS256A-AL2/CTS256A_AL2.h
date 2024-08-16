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

#pragma once

#include "System_I.h"

#include "SystemConsole.h"
#include "Ram.h"
#include "Rom.h"
#include "InOut_I.h"
#include "TMS7000CPU.h"
#include "TMS7000Disassembler.h"
#include "ConIOConsole.h"

#include <iostream>

// Number of READs after last input/output before entering DEBUG mode
#define DEBUG_CTR_RELOAD 999999

// Number of READs after eof and last output before stopping the emulation
#define EOF_CTR_RELOAD 199999

class CTS256A_AL2_Data_InOut
	: public Memory_I, public InOut_I
{
public:
	CTS256A_AL2_Data_InOut( TMS7000CPU &cpu, std::istream &istr, std::ostream &ostr )
		: cpu_( cpu ), istr_( istr ), ostr_( ostr ), bport_( 0 ), initctr_( 6 ), irq3ctr_( 0 ), eof_( false )
		, debug_( false ), debug_rules_( false ), verbose_( false ), echo_( false ), noOK_( false ), mode_( 'T' ), debugctr_( DEBUG_CTR_RELOAD )
	{
		memset( ram_, 0, 0x800 );
	}

	uchar read( ushort addr );

	uchar write( ushort addr, uchar data );

    reader_t getReader()
	{
		return 0;
	}

    writer_t getWriter()
	{
		return 0;
	}

    void* getObject()
	{
		return 0;
	}

	// out char
	virtual uchar out( ushort addr, uchar data );

	// in char
	virtual uchar in( ushort addr );

	void setOption( uchar option, uint value );

	uint getOption( uchar option );

	void debug_rule();

private:
	uchar					bport_;
	TMS7000CPU				&cpu_;
	uchar					ram_[0x800];
	std::istream			&istr_;
	std::ostream			&ostr_;
	uchar					initctr_;
	ushort					irq3ctr_;
	uint					debugctr_;
	uint					eofctr_;
	bool					eof_;
	bool					debug_;
	bool					debug_rules_;
	bool					verbose_;
	bool					echo_;
	bool					textMode_;
	bool					noOK_;
	char					mode_;
	char					initial_;
};


class CTS256A_AL2 : public System_I
{
public:
	CTS256A_AL2( std::istream &istr, std::ostream &ostr )
	: debug_( false ), istr_( istr), ostr_( ostr ), data_( cpu_, istr, ostr )
	{
		systemConsole_.setSystem( this );
		systemConsole_.setConsole( &console_ );
		cpu_.setExtMemory( &data_ );
		cpu_.setExtInOut( &data_ );
		cpu_.setConsole( &systemConsole_ );
		cpu_.setMode( &mode_ );
		disass_.setCode( &data_ );
	}

	~CTS256A_AL2(void)
	{
	}

	void run();
	void stop();
	void exit();
	void reset();
	void wakeup();
	void step();
	void callstep();

	void setOption( uchar option, uint value )
	{
		data_.setOption( option, value );
		if ( option == 'D' )
			debug_ = value != 0;
	}

private:
	TMS7000CPU				cpu_;
	CTS256A_AL2_Data_InOut	data_;
	Mode					mode_;
	ConIOConsole			console_;
	SystemConsole			systemConsole_;
	TMS7000Disassembler		disass_;
	bool					debug_;
	std::istream			&istr_;
	std::ostream			&ostr_;
};
