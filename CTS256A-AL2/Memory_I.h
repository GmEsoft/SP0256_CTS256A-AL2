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
