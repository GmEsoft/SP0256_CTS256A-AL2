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
	virtual uchar write( ushort addr, uchar data )
	{
		return data;
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
