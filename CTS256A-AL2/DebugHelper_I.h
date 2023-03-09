/*
    CTS256A-AL2 - CPU Debugger Helper API.

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

#include "runtime.h"
#include "Memory_I.h"
#include "Console_I.h"

class DebugHelper_I
{
public:
	DebugHelper_I(void){}
	virtual ~DebugHelper_I(void){}
	virtual uint getPC() = 0;
	virtual uint getSP() = 0;
	virtual void sim() = 0;
	virtual Memory_I &getData() = 0;
	virtual void printRegNamesLine( Console_I &console ) = 0;
	virtual void printRegsLine( Console_I &console ) = 0;
	virtual void printSource( Console_I &console, uint &pc ) = 0;
	virtual void printTabSourceWidth( Console_I &console ) = 0;
	virtual const char *getSource( uint &pc ) = 0;
	virtual uchar getSourceByte( uint pc ) = 0;
	virtual const char *getPointer( int n, uint &ptr ) = 0;
	virtual bool isCall( uint pc ) = 0;
	virtual bool isRet( uint pc ) = 0;
	virtual bool isSetSP( uint pc ) = 0;
	virtual bool isBreak( uint pc ) = 0;
	virtual bool isBreakRet( uint lastpc, uint retsp ) = 0;

};

