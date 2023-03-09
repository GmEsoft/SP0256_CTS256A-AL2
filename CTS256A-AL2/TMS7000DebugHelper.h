/*
    CTS256A-AL2 - TMS7000 Debugger Helper.

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

#include "DebugHelper_I.h"
#include "TMS7000CPU.h"
#include "TMS7000Disassembler.h"

class TMS7000DebugHelper : public DebugHelper_I
{
public:
	TMS7000DebugHelper( TMS7000CPU &cpu, TMS7000Disassembler &disass )
		: cpu_( cpu ), disass_( disass ), data_( cpu.getData() ), sp_( cpu.getSp() ), flags_( cpu.getFlags() )
	{
	}

	~TMS7000DebugHelper(void)
	{
	}

	uint getPC()
	{
		return cpu_.getPC();
	}

	uint getSP()
	{
		return sp_;
	}

	void sim()
	{
		cpu_.sim();
	}

	Memory_I &getData()
	{
		return *cpu_.getMemory();
	}

	void printRegNamesLine( Console_I &console );

	void printRegsLine( Console_I &console );

	void printSource( Console_I &console, uint &pc );

	void printTabSourceWidth( Console_I &console );

	const char *getSource( uint &pc );

	uchar getSourceByte( uint pc );

	const char *getPointer( int n, uint &ptr );

	bool isCall( uint pc );

	bool isRet( uint pc );

	bool isSetSP( uint pc );

	bool isBreak( uint pc );

	bool isBreakRet( uint lastpc, uint retsp );

private:
	TMS7000CPU				&cpu_;
	TMS7000Disassembler		&disass_;
	uchar					*data_;
	st_t					&flags_;
	uchar					&sp_;
};

