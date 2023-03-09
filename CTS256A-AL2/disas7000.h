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

#ifndef __DISAS7000_H__
#define __DISAS7000_H__

#include "TMS7000CPU.h"

#include "Symbols.h"
#include "runtime.h"



typedef int (*compfptr_t)(const void*, const void*);
typedef uchar (*readfptr_t)( ushort );
typedef uchar (*writefptr_t)( ushort, uchar );


// get label of given code address
char* getLabel( uint val, char ds );

// set label generated (DS labels)
void setLabelGen( uint val );

// get label of given code address
char* getXAddr( uint val );

// get comment associated to label of given code address from last getXAddr()/getLabel() call
char* getLastComment();

// fetch long external address and return it as hex string or as label
char* getLAddr();

// get single instruction source
char* source();

// Z-80 simulator
extern uint		pc;

extern char		noNewEqu;

extern uint		nTms7000Symbols;
extern symbol_t	*tms7000Symbols;
extern int		pcOffset;
extern ushort	pcOffsetBeg, pcOffsetEnd;
extern char		pcOffsetSeg;

extern instr_t instr[];

// Attach TMS7000 to external symbol table
void setTms7000Symbols( symbol_t *pSymbols, int pNSymbols, int pSymbolsSize );

void updateTms7000Symbols();

void resetTms7000Symbols();

// Attach TMS7000 to memory and I/O ports
void setTms7000MemIO( readfptr_t getdata );

// Sort symbols
int  symSort(const void *a, const void *b);

// Compare symbols by name
int  symCompName(symbol_t *a, symbol_t *b);

#endif
