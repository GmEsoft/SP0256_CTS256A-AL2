/*
    CTS256A-AL2 - CPU Disassembler API.

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

// DISASSEMBLER API

#include "runtime.h"
#include "Symbols.h"
#include "Memory_I.h"

class Disassembler
{
public:
	Disassembler() : pc_( 0 ), symbols_( 0 ), code_( 0 )
	{
	}

	// destructor
	virtual ~Disassembler()
	{
	}

	// set PC
	void setPC( uint pc )
	{
		pc_ = pc;
	}

	// get PC
	uint getPC()
	{
		return pc_;
	}

	// fetch
	uchar fetch()
	{
		return code_ ? code_->read( ushort( pc_++ ) ) : 0;
	}

	// set symbols table
	void setSymbols( Symbols *symbols )
	{
		symbols_ = symbols;
	}

	// Set Code Handler
	void setCode( Memory_I *mem )
	{
		code_ = mem;
	}

	// get single instruction source
	virtual const char* source () = 0;

protected:
	Memory_I	*code_;
	uint	    pc_;
	Symbols	    *symbols_;

};
