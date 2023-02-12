#pragma once

// TMS7000 CPU

#include "CPU.h"
#include "ConsoleProxy.h"
#include "InOut_I.h"

class TMS7000CPU;

struct instr_t
{
	int mnemon, opn1, opn2;
};


///	bit-mapping of PSW register
struct st_t { unsigned b0:1, b1:1, b2:1, b3:1, i:1, z:1, n:1, c:1; };

class TMS7000CPU :
	public CPU, public ConsoleProxy, public Memory_I, public InOut_I
{
public:
	TMS7000CPU() : CPU()
	{
		init();
	}

	~TMS7000CPU();

	// CPU abstract class

	// Trigger IRQ interrupt
	virtual void trigIRQ( char irq );

	// Get IRQ status
	virtual char getIRQ( void );

	// Trigger NMI interrupt
	virtual void trigNMI( void );

	// Get NMI status
	virtual char getNMI( void );

	// CPU Reset
	virtual void reset();

	// Memory_I interface
	// write char
	virtual uchar write( ushort addr, uchar data )
	{
		if ( addr < 0x100 )
			this->data[addr] = data;
		else if ( addr >= 0x200 && this->pExtData_ )
			this->pExtData_->write( addr, data );
		return data;
	}

	// read char
	virtual uchar read( ushort addr ) 
	{
		if ( addr < 0x100 )
			return this->data[addr];
		else if ( addr >= 0x200 && this->pExtData_ )
			return this->pExtData_->read( addr );
		return 0xFF;
	}

    // get reader
    virtual reader_t getReader()
	{
		return 0;
	}
    
    // get writer
    virtual writer_t getWriter()
	{
		return 0;
	}
    
    // get object
    virtual void* getObject()
	{
		return 0;
	}

	// InOut_I interface
	// out char
	virtual uchar out( ushort addr, uchar data );

	// in char
	virtual uchar in( ushort addr );



	// Init internal pointers
	void init();


	// Get Parity
	uchar parity( const uchar x);

	// Get Data
	uchar* getData()
	{
		return data;
	}

	// Set External Memory
	void setExtMemory( Memory_I *pExtData )
	{
		pExtData_ = pExtData;
		this->setMemory( this );
		this->setCode( this );
	}

	// Set External Memory
	void setExtInOut( InOut_I *pExtInOut )
	{
		pExtInOut_ = pExtInOut;
		this->setInOut( this );
	}

	// Get Flags
	st_t& getFlags()
	{
		return *pSt;
	}

	uchar& getSp()
	{
		return sp;
	}

	uint laddr()
	{
		uint x;
		x = fetch() << 8;
		x += fetch();
		return x;
	}

	// Get Short Relative Code Address
	uint saddr()
	{
		signed char d;
		d = fetch();
		return pc_ + d;
	}


	void simtimers();

	void simintdetect();

	void simintprocess();

	// Execute 1 Statement
	void sim();

	void simop( const uchar opcode );

	void stop();

public:
	static instr_t	instr[];

protected:

private:
	long			cycles;
	ushort			opcode;
	uchar			irq/*, nmi*/;
	uchar			data[256];
	uchar			*a, *b;
	uchar			sp, st;
	st_t			*pSt;
	Memory_I		*pExtData_;
	InOut_I			*pExtInOut_;
	ushort			pc0_;

	// Internal Peripherals
	uchar			iocnt0_;				///< P0
	uchar			iocnt1_;				///< P16

	uchar			intblocked;				///< Interrupt handling blocked( write to IE or IP )
};

