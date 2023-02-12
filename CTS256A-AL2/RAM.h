#pragma once

// RAM ACCESSOR

#define USE_RAM_ACCESSORS 1

#include "Memory_I.h"

#if USE_RAM_ACCESSORS
static uchar RAM_read( void *object, ushort addr );
static uchar RAM_write( void *object, ushort addr, uchar data );
#endif

class RAM : public Memory_I
{
public:
	RAM()
		: buffer_( 0 ), size_( 0 )
	{
	}

    // get writer
    virtual writer_t getWriter()
    {
#if USE_RAM_ACCESSORS
		return RAM_write;
#else
		return 0;
#endif
	}

    // get reader
    virtual reader_t getReader()
    {
#if USE_RAM_ACCESSORS
        return RAM_read;
#else
		return 0;
#endif
    }

    // get object
    virtual void* getObject()
    {
#if USE_RAM_ACCESSORS
        return this;
#else
		return 0;
#endif
    }
    
	void setBuffer( uchar *_buffer, ushort _size )
	{
		buffer_= _buffer; 
		size_ = _size;
	}

	uchar *getBuffer()
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
		if ( !size_ || addr < size_ )
			buffer_[addr] = data;
		return data;
	}

	// read char
	virtual uchar read( ushort addr )
	{
		if ( !size_ || addr < size_ )
			return buffer_[addr];
		return 0;
	}

#if USE_RAM_ACCESSORS
	friend uchar RAM_read(void*, ushort);
	friend uchar RAM_write(void*, ushort, uchar);
#endif

private:
	uchar *buffer_;
	ushort size_;
};

#if USE_RAM_ACCESSORS
static uchar RAM_read( void *object, ushort addr )
{
	RAM &ram = *(RAM*)object;
	if ( !ram.size_ || addr < ram.size_ )
		return ram.buffer_[addr];
	return 0;
}

static uchar RAM_write( void *object, ushort addr, uchar data )
{
	RAM &ram = *(RAM*)object;
	if ( !ram.size_ || addr < ram.size_ )
		ram.buffer_[addr] = data;
	return data;
}
#endif
