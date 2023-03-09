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

#pragma once

#include "disassembler.h"

class TMS7000Disassembler :
	public Disassembler
{
public:
	TMS7000Disassembler(void)
	{
	}

	~TMS7000Disassembler(void)
	{
	}

	//  return hex-string or label for double-byte x(dasm)
	const char *getxaddr(uint x);

	//	return internal byte address as label or as hex string
	const char *getdataaddr();

	//  return internal bit address as BIT name or as hex string
	const char *getbitaddr();

	// fetch long external address and return it as hex string or as label
	const char *getladdr();

	// fetch absolute segment external address and return it as hex string or as label
	const char *getaaddr(char opcode);

	// fetch short relative external address and return it as hex string or as label
	const char *getsaddr();

	// return operand name or value
	const char *getoperand(int opn);

	// get 1st operand name or value
	const char *getoperand1(int opcode);

	// get 2nd operand name or value
	const char *getoperand2(int opcode);

	// get single instruction source
	virtual const char *source();
};

