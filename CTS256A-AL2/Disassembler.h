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
		return code_ ? code_->read( pc_++ ) : 0;
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
