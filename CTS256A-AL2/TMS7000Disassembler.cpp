/*
    CTS256A-AL2 - TMS7000 Disassembler.

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

#include "TMS7000Disassembler.h"
#include "disas7000.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <io.h>
#include <search.h>



//  return hex-string or label for double-byte x( dasm )
const char *TMS7000Disassembler::getxaddr( uint x )
{
	return getXAddr( x );
}


//	return internal byte address as label or as hex string
const char *TMS7000Disassembler::getdataaddr()
{
	return "";
}

//  return internal bit address as BIT name or as hex string
const char *TMS7000Disassembler::getbitaddr()
{
	return "";
}

// fetch long external address and return it as hex string or as label
const char *TMS7000Disassembler::getladdr()
{
	return getLAddr();
}

// fetch absolute segment external address and return it as hex string or as label
const char *TMS7000Disassembler::getaaddr( char opcode )
{
	return "";
}

// fetch short relative external address and return it as hex string or as label
const char *TMS7000Disassembler::getsaddr()
{
	return "";//getSAddr();
}

// return operand name or value
const char *TMS7000Disassembler::getoperand( int opn )
{
	return "";//getOperand();
}

// get 1st operand name or value
const char *TMS7000Disassembler::getoperand1( int opcode )
{
	return "";//getOperand();
}

// get 2nd operand name or value
const char *TMS7000Disassembler::getoperand2( int opcode )
{
	return "";//getOperand();
}


// get single instruction source
const char *TMS7000Disassembler::source()
{
	::pc = pc_;
	const char *s = ::source();
	pc_ =::pc;
	return s;
}

