/*
    CTS256A-AL2 - CPU Symbols Table.

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

// SYMBOLS TABLE

#include "runtime.h"

struct symbol_t 
{
	char    name[41];
    uint    val;
    char    seg;
    uint  	call;
};

class Symbols
{
public:
	// Attach Z80 to external symbol table
	void setSymbols( symbol_t *pSymbols, int pNSymbols )
	{
		symbols_ = pSymbols;
		nSymbols_ = pNSymbols;
	}

	// Sort symbols
	static int  compVal(symbol_t *a, symbol_t *b);

	// Compare symbols by name
	static int  compName(symbol_t *a, symbol_t *b);

	// get symbol
	symbol_t* getSymbol( char seg, uint val );

	// get label of given code address
	char* getLabel(uint val);

	// get label and offset of given code address
	char* getLabelOffset(uint val);

private:
	symbol_t	*symbols_;
	int			nSymbols_;
};
