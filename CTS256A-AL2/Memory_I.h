/*
    CTS256A-AL2 - CPU Memory API.

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

// MEMORY API

#include "runtime.h"

class Memory_I
{
public:
    typedef uchar (*writer_t)( void *object, ushort addr, uchar data );
    typedef uchar (*reader_t)( void *object, ushort addr );

	// write char
	virtual uchar write( ushort addr, uchar data ) = 0;

	// read char
	virtual uchar read( ushort addr ) = 0;

    // get reader
    virtual reader_t getReader() = 0;
    
    // get writer
    virtual writer_t getWriter() = 0;
    
    // get object
    virtual void* getObject() = 0;
    
	// destructor
	virtual ~Memory_I()
	{
	}
};
