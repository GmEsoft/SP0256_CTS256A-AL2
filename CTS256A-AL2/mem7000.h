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

#ifndef __MEM51_H__
#define __MEM51_H__

#define SYMSIZE 10240        // symbol table size


typedef struct
{
	unsigned int beg, end;
} range_t;

extern range_t ranges[];
extern unsigned nranges;

// Data Write Routine (memory address space)
unsigned char putdata( unsigned short addr, unsigned char byte );

// Data Read Routine (memory address space)
unsigned char getdata( unsigned short addr );


#endif
