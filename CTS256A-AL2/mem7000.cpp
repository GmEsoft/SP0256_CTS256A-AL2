/*
    CTS256A-AL2 - TMS7000 Memory.

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

//#define NDEBUG
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "mem7000.h"
#include "disas7000.h"

#ifdef trace
#undef trace
#endif

#ifdef _TRACE_ACTIVE_
#	define trace(trace) trace
#	define tgetch() getch()
#else
#	define trace(trace)
#	define tgetch()
#endif




FILE *readfile(const char *prompt, const char *mode);

char data[0x10000];

range_t ranges[1000];
unsigned nranges = 0;

// Data Write Routine (memory address space)
unsigned char putdata(unsigned short addr, unsigned char byte)
{
	// M1 System RAM
	data[addr] = byte;

	if ( !nranges || ranges[nranges-1].end != addr )
	{
		ranges[nranges].beg = addr;
		++nranges;
	}
	ranges[nranges-1].end = addr + 1;

    return byte;
}

// TRS-80 Data Read Routine (memory address space)
unsigned char getdata(unsigned short addr)
{
	unsigned char byte;

    byte = data[addr];
    return (unsigned char)byte;
}

