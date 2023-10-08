/*
    CTS256A-AL2 - ROM Accessor.

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

// ROM ACCESSOR

#include "Memory_I.h"

class ROM : public Memory_I
{
public:
	ROM()
		: buffer_( 0 ), size_( 0 )
	{
	}

    // get writer
    virtual writer_t getWriter()
    {
		return 0;
    }

    // get reader
    virtual reader_t getReader()
    {
        return 0;
    }

    // get object
    virtual void* getObject()
    {
        return 0;
    }
    
	void setBuffer( const uchar *_buffer, ushort _size )
	{
		buffer_= _buffer; 
		size_ = _size;
	}

	const uchar *getBuffer()
	{
		return buffer_;
	}
	
	ushort getSize()
	{
		return size_;
	}

	// write char
	virtual uchar write( ushort /*addr*/, uchar byte )
	{
		return byte;
	}

	// read char
	virtual uchar read( ushort addr )
	{
		if ( !size_ || addr < size_ )
			return buffer_[addr];
		return 0;
	}

private:
	const uchar *buffer_;
	ushort size_;
};
