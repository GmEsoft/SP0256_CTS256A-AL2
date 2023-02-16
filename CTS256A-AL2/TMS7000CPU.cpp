#pragma warning(disable:4244)	// warning C4244: '%0' : conversion from '%1' to '%2', possible loss of data

#include "TMS7000CPU.h"
#include <assert.h>
#include <cstring>

//  Enumerated constants for instructions, also array subscripts
enum {
	ADC=0,	ADD,	AND,	ANDP,	BTJO,	BTJOP,	BTJZ,	BTJZP,
	BR,		CALL,	CLR,	CLRC,	CMP,	CMPA,	DAC,	DEC,
	DECD,	DINT,	DJNZ,	DSB,	EINT,	IDLE,	INC,	INV,
	JMP,	JN,		JZ,		JC,		JP,		JPZ,	JNZ,	JNC,
	LDA,	LDSP,	MOV,	MOVD,	MOVP,	MPY,	NOP,	OR,
	ORP,	POP,	PUSH,	RETI,	RETS,	RL,		RLC,	RR,
	RRC,	SBB,	SETC,	STA,	STSP,	SUB,	SWAP,	TRAP,
	TSTA,	TSTB,	XCHB,	XOR,	XORP,	DB
	};


//  Enumerated constants for operands, also array subscripts
enum {		// operand (0=none)
	A=1, 	// A
	B, 		// B
	RN, 	// Rn
	PN, 	// Pn
	BYTE,	// %>byte
	WORD,	// %>word
	WORD_B, // %>word(B)
	OFST, 	// PC+offs
	ADDR, 	// @>addr
	ADDR_B, // @>addr(B)
	ATRN, 	// *Rn
	ST, 	// ST
	N, 		// ??
	NTRAP, 	// TRAP n
	OPCODE	// DB opcode
	};


TMS7000CPU::~TMS7000CPU()
{
}

// Init internal pointers
void TMS7000CPU::init( void )
{
	pSt		= (st_t*)&st;
	a		= &data[0];
	b		= &data[1];
	std::memset( data, 0, sizeof data );
	reset();
}


// CPU Reset
void TMS7000CPU::reset( void )
{
	// 1) All 0s are written to the Status Register. This clears the global interrupt
	//    enable bit (I), disabling all interrupts.
	st = 0;

	// 2) All Os are written to the IOCNTO register. This disables INT1*, INT2, and
	//    INT3* and leaves the INTn flag bits unchanged.
	iocnt0_ = 0;	

	// 3) All Os are written to the IOCNT1 register in the TMS70x2 and
	//    TMS70Cx2 devices. This disables INT4 and INT5.
	iocnt1_ = 0;	

	// 4) The PC's MSB and LSB values before RESET* was asserted are stored in
	//    R0 and R1 (registers A and B), respectively.
	*a = pc_ >> 8;
	*b = pc_ & 0xFF;
	
	// 5) The Stack Pointer is initialized to >01.
	sp = 1;
	
	// 6) The MSB and LSB of the RESET interrupt vector are fetched from locations
	//    >FFFE and >FFFF, respectively (see Table 3-13, page 3-26),
	//    and loaded into the Program Counter.
	pc_ = ( getdata( 0xFFFE ) << 8 ) | getdata( 0xFFFF );

	cycles = 0;
}


// Trigger IRQ interrupt
void TMS7000CPU::trigIRQ( char myirq )
{
	irq |= myirq;
}

// Get IRQ status
char TMS7000CPU::getIRQ( void )
{
	return irq;
}

// Trigger NMI interrupt
void TMS7000CPU::trigNMI( void )
{
}

// Get NMI status
char TMS7000CPU::getNMI( void )
{
	return 0;
}

// InOut_I interface
// out char
uchar TMS7000CPU::out( ushort addr, uchar data )
{
	switch ( addr )
	{
	case 0:	// IOCNT0
		iocnt0_ = ( data & ~0x2A ) | ( iocnt0_ & 0x2A & ~data );
		break;
	case 16:// IOCNT1
		iocnt1_ = ( data & ~0xFA ) | ( iocnt1_ & 0x0A & ~data );
		break;
	default:
		if ( pExtInOut_ )
			pExtInOut_->out( addr, data );
	}
	return data;
}

// in char
uchar TMS7000CPU::in( ushort addr )
{
	switch ( addr )
	{
	case 0:	// IOCNT0
		return iocnt0_;
		break;
	case 1:	// IOCNT1
		return iocnt1_;
		break;
	default:
		if ( pExtInOut_ )
			return pExtInOut_->in( addr );
	}
	return 0xFF;
}



// Get Parity of ACC (1=even, 0=odd)
uchar TMS7000CPU::parity( const uchar x )
{
	uchar res = x;
	res ^= res >> 4;
	res ^= res >> 2;
	res ^= res >> 1;
	return res & 1;
}


void TMS7000CPU::simtimers()
{
}

void TMS7000CPU::simintdetect()
{
	if ( irq & 0x02 ) // IRQ1
	{
		iocnt0_ |= 0x02; // raise IRQ1*
		irq &= ~0x02;
	}
	if ( irq & 0x08 ) // IRQ3
	{
		iocnt0_ |= 0x20; // raise IRQ3*
		irq &= ~0x08;
	}
}

void TMS7000CPU::simintprocess()
{
	if ( pSt->i )
	{
		ushort itrap;
		if ( ( iocnt0_ & 0x03 ) == 0x03 )
			itrap = 1;
		else if ( ( iocnt0_ & 0x30 ) == 0x30 )
			itrap = 3/*, stop()*/;
		else
			return;
		
		itrap = 0xFFFE - ( itrap << 1 );
		data[++sp] = st;
		data[++sp] = pc_ >> 8;
		data[++sp] = pc_ & 0xFF;
		pSt->i = 0;
		pc_ = ( getdata( itrap ) << 8 ) | getdata( itrap+1 );
	}	
	return;

}

// Execute 1 Statement
void TMS7000CPU::sim()
{
	pc0_ = pc_;

	// Fetch opcode
	opcode = fetch();

	intblocked = 0;

	// Execute opcode
	this->simop( opcode );

	// Update timers
	simtimers();

	// Detect interrupts
	simintdetect();

	// Process interrupts
	simintprocess();

	runcycles( cycles );
	cycles = 0;
}

void TMS7000CPU::simop( const uchar opcode )
{
	const instr_t &instr = this->instr[opcode];
	uchar *pOpn1 = 0, *pOpn2 = 0;
	uchar opn1, opn2, byte;
	ushort res;
	ushort word;

	switch ( instr.opn1 )
	{
	case 0:			// No operand
		break;
	case A:			// A
		pOpn1 = this->a;
		break;
	case B:			// B
		pOpn1 = this->b;
		break;
	case RN:		// Rn
		pOpn1 = this->data + this->fetch();
		break;
	case PN:		// Pn
		opn1 = fetch();
		break;
	case BYTE:		// %>byte
		opn1 = fetch();
		break;
	case WORD:		// %>word
		word = laddr();
		break;
	case WORD_B:	// %>word(B)
		word = laddr()+*b;
		break;
	case OFST:		// PC+offs
		word = saddr();
		break;
	case ADDR:		// @>addr
		word = laddr();
		break;
	case ADDR_B:	// &>addr(B)
		word = laddr()+*b;
		break;
	case ATRN:		// *Rn
		byte = fetch();
		word = ( data[uchar(byte-1)] << 8 ) + data[byte];
		break;
	case ST: 		// ST
		opn1 = this->st;
		break;
	case N: 		// ??
		break;
	case NTRAP: 	// TRAP n
		word = 0xFFFE - ( ( 0xFF - opcode ) << 1 );
		word = ( this->getdata( word ) << 8 ) | this->getdata( word + 1 );
		break;
	case OPCODE:	// DB opcode
		stop();
		break;
	default:
		stop();
	}

	if ( pOpn1 )
		opn1 = *pOpn1;

	switch ( instr.opn2 )
	{
	case 0:			// No operand
		break;
	case A:			// A
		pOpn2 = this->a;
		break;
	case B:			// B
		pOpn2 = this->b;
		break;
	case RN:		// Rn
		pOpn2 = this->data + this->fetch();
		break;
	case PN:		// Pn
		opn2 = fetch();
		break;
	case BYTE:		// %>byte
		opn2 = fetch();
		break;
	case WORD:		// %>word
		word = laddr();
		break;
	case WORD_B:	// %>word(B)
		word = laddr()+*b;
		break;
	case OFST:		// PC+offs
		word = saddr();
		break;
	case ADDR:		// @>addr
		word = laddr();
		break;
	case ADDR_B:	// &>addr(B)
		word = laddr()+*b;
		break;
	case ATRN:		// *Rn
		byte = fetch();
		word = ( data[uchar(byte-1)] << 8 ) + data[byte];
		break;
	case ST: 		// ST
		opn2 = this->st;
		break;
	case N: 		// ??
		break;
	case NTRAP: 	// TRAP n
		word = 0xFFFE - ( ( 0xFF - opcode ) << 1 );
		word = ( this->getdata( word ) << 8 ) | this->getdata( word + 1 );
		break;
	case OPCODE:	// DB opcode
		stop();
		break;
	default:
		stop();
	}

	if ( pOpn2 )
		opn2 = *pOpn2;

	// Instructions not yet implemented have a "stop();" line.
	switch ( instr.mnemon )
	{
	case ADC:	// Add with carry
		res = opn2 + opn1 + pSt->c;
		opn2 = res;
		pSt->c = ( res >> 8 ) & 1;
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = opn2 == 0;
		pOpn1 = 0;
		break;
	case ADD:	// Add
		res = opn2 + opn1;
		opn2 = res;
		pSt->c = ( res >> 8 ) & 1;
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = opn2 == 0;
		pOpn1 = 0;
		break;
	case AND:	// Logical AND
		res = opn2 & opn1;
		opn2 = res;
		pSt->c = 0;
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = opn2 == 0;
		pOpn1 = 0;
		break;
	case ANDP:	// AND peripheral register
		res = indata( opn2 ) & opn1;
		outdata( opn2, res );
		pSt->c = 0;
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = opn2 == 0;
		pOpn1 = 0;
		break;
	case BTJO:	// Bit test and jump if one
		word = saddr();
		if ( opn1 & opn2 )
			pc_ = word;
		pOpn1 = pOpn2 = 0;
		break;
	case BTJOP:
		stop();
		break;
	case BTJZ:	// Bit test and jump if zero
		word = saddr();
		if ( opn1 & ~opn2 )
			pc_ = word;
		pOpn1 = pOpn2 = 0;
		break;
	case BTJZP:
		stop();
		break;
	case BR:	// Branch
		pc_ = word;
		break;
	case CALL:
		data[++sp] = pc_ >> 8;
		data[++sp] = pc_ & 0xFF;
		pc_ = word;
		break;
	case CLR:	// Clear
		opn1 = 0;
		pSt->c = 0;
		pSt->n = 0;
		pSt->z = 1;
		break;
	case CLRC:
		stop();
		break;
	case CMP:	// Compare
		res = opn2 - opn1;
		pSt->c = ( res >> 8 ) ^ 1; // !! c == 0 if borrow !!
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = res == 0;
		pOpn1 = pOpn2 = 0;
		break;
	case CMPA:
		res = read( word );
		res = *a - res;
		pSt->c = ( res >> 8 ) ^ 1; // !! c == 0 if borrow !!
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = res == 0;
		pOpn1 = pOpn2 = 0;
		break;
	case DAC:
		stop();
		break;
	case DEC:	// Decrement
		--opn1;
		pSt->c = opn1 != 0xFF;
		pSt->n = ( opn1 >> 7 ) & 1;
		pSt->z = ( opn1 == 0 );
		break;
	case DECD:	// Decrement double
		--opn1;
		--pOpn1;
		if ( opn1 == 0xFF )
		{
			--*pOpn1;
			pSt->c = *pOpn1 != 0xFF;
		}
		pSt->n = ( *pOpn1 >> 7 ) & 1;
		pSt->z = ( *pOpn1 == 0 );
		++pOpn1;
		break;
	case DINT:
		stop();
		break;
	case DJNZ:
		stop();
		break;
	case DSB:
		stop();
		break;
	case EINT:	// Enable interrupts
		st |= 0xF0;
		break;
	case IDLE:
		stop();
		break;
	case INC:	// Increment
		++opn1;
		pSt->c = pSt->z = ( opn1 == 0 );
		pSt->n = ( opn1 >> 7 ) & 1;
		break;
	case INV:
		stop();
		break;
	case JMP:	// Jump Unconditional
		pc_ = word;
		break;
	case JN:	// Jump if negative (CNZ=x1x)
		if ( pSt->n )
			pc_ = word;
		break;
	case JZ:	// Jump if zero <=> JEQ=Jump if equal (CNZ=xx1)
		if ( pSt->z )
			pc_ = word;
		break;
	case JC:	// Jump if carry <=> JHS=Jump if higher or same (CNZ=1xx)
		stop();
		break;
	case JP:	// Jump if positive (CNZ=x00)
		if ( !pSt->n && !pSt->z )
			pc_ = word;
		break;
	case JPZ:	// Jump if positive or zero (CNZ=x0x)
		if ( !pSt->n )
			pc_ = word;
		break;
	case JNZ:	// Jump if non-zero <=> JNE: Jump if not equal (CNZ=xx0)
		if ( !pSt->z )
			pc_ = word;
		break;
	case JNC:	// Jump if no carry <=> JL=Jump if lower (CNZ=0xx)
		if ( !pSt->c )
			pc_ = word;
		break;
	case LDA:	// Load register A
		*a = res = read( word );
		pSt->c = 0;
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = res == 0;
		break;
	case LDSP:	// Load Stack Pointer
		this->sp = *b;
		break;
	case MOV:	// Move
		opn2 = opn1;
		pSt->c = 0;
		pSt->n = ( opn2 >> 7 ) & 1;
		pSt->z = opn2 == 0;
		pOpn1 = 0;
		break;
	case MOVD:	// Move double
		if ( pOpn1 )
		{
			word = ( *(pOpn1-1) << 8 ) | *pOpn1;
			pOpn1 = 0;
		}

		if ( pOpn2 ) 
		{
			*(pOpn2-1) = res = word >> 8; 
			*pOpn2 = word & 0xFF;
			pOpn2 = 0;
			pSt->c = 0;
			pSt->n = ( res >> 7 ) & 1;
			pSt->z = res == 0;
		}
		else
		{
			stop(); // should not happen
		}

		break;
	case MOVP:	// Move to/from peripheral register
		if ( instr.opn1 == PN )
			opn1 = this->indata( opn1 );
		
		pSt->c = 0;
		pSt->n = ( opn1 >> 7 ) & 1;
		pSt->z = opn1 == 0;
		
		if ( instr.opn2 == PN )
		{
			this->outdata( opn2, opn1 );
			pOpn2 = 0;
		}
		else
		{
			opn2 = opn1;
		}
		
		pOpn1 = 0;
		break;
	case MPY:	// Multiply
		res = opn1 * opn2;
		*a = res >> 8;
		*b = res & 0xFF;
		pSt->c = 0;
		pSt->n = ( *a >> 7 ) & 1;
		pSt->z = !*a;
		pOpn1 = pOpn2 = 0;
		break;
	case NOP:
		stop();
		break;
	case OR:	// Logical OR
		res = opn2 | opn1;
		opn2 = res;
		pSt->c = 0;
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = opn2 == 0;
		pOpn1 = 0;
		break;
	case ORP:	// OR peripheral register
		res = indata( opn2 ) | opn1;
		outdata( opn2, res );
		pSt->c = 0;
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = opn2 == 0;
		pOpn1 = 0;
		break;
	case POP:	// Pop from stack
		opn1 = data[sp--];
		break;
	case PUSH:	// Push on stack
		data[++sp] = opn1;
		pOpn1 = 0;
		break;
	case RETI:	// Return from interrupt
		pc_ = data[sp--];
		pc_ |= data[sp--] << 8;
		st = data[sp--];
		break;
	case RETS:	// Return from subroutine
		pc_ = data[sp--];
		pc_ |= data[sp--] << 8;
		break;
	case RL:
		stop();
		break;
	case RLC:
		stop();
		break;
	case RR:
		stop();
		break;
	case RRC:	// Rotate right through carry
		res = ( opn1 >> 1 ) | ( pSt->c << 7 );
		pSt->c = opn1 & 1;
		opn1 = res;
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = res == 0;
		break;
	case SBB:	// Subtract with borrow
		res = opn2 - opn1 - 1 + pSt->c;
		opn2 = res;
		pSt->c = ( res >> 8 ) ^ 1; // !! c == 0 if borrow !!
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = res == 0;
		pOpn1 = 0;
		break;
	case SETC:
		stop();
		break;
	case STA:	// Store register A
		write( word, res = *a );
		pSt->c = 0;
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = res == 0;
		break;
	case STSP:
		stop();
		break;
	case SUB:	// Subtract
		res = opn2 - opn1;
		opn2 = res;
		pSt->c = ( res >> 8 ) ^ 1; // !! c == 0 if borrow !!
		pSt->n = ( res >> 7 ) & 1;
		pSt->z = res == 0;
		pOpn1 = 0;
		break;
	case SWAP:
		opn1 = ( opn1 >> 4 ) | ( opn1 << 4 );
		pSt->c = opn1 & 1;
		pSt->n = ( opn1 >> 7 ) & 1;
		pSt->z = !opn1;
		break;
	case TRAP:
		stop();
		break;
	case TSTA:	// Test register A <=> CLRC=Clear carry
		pSt->c = 0;
		pSt->n = *a >> 7;
		pSt->z = !*a;
		break;
	case TSTB:
		stop();
		break;
	case XCHB:
		stop();
		break;
	case XOR:
		stop();
		break;
	case XORP:
		stop();
		break;
	case DB:
		stop();
		break;
	default:
		stop();
	}

	if ( pOpn1 )
		*pOpn1 = opn1;

	if ( pOpn2 )
		*pOpn2 = opn2;

}

// Stop emulation in case of invalid or non-implemented instructions.
void TMS7000CPU::stop()
{
	pc_ = pc0_;
	this->printf( "\nTMS7000 Stopped at %04X\n", pc_ );
	this->setMode( MODE_STOP );
}

// PROCESSOR INSTRUCTIONS TABLE ///////////////////////////////////////////////

//  Processor's instruction set
instr_t TMS7000CPU::instr[] = {
//		mnemon,			opn1,			opn2
// 00-0F
		NOP,            0,              0,
		IDLE,           0,              0,
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		EINT,			0,				0,
		DINT,           0,				0,
		SETC,           0,				0,
		POP,            ST,             0,
		STSP,           0,				0,
		RETS,           0,				0,
		RETI,           0,				0,
		DB,				OPCODE,         0,
		LDSP,           0,				0,
		PUSH,           ST,             0,
		DB,				OPCODE,         0,
// 10-1F
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		MOV,			RN,				A,
		AND,            RN,				A,
		OR,				RN,				A,
		XOR,            RN,				A,
		BTJO,           RN,				A,
		BTJZ,           RN,				A,
		ADD,            RN,				A,
		ADC,            RN,				A,
		SUB,            RN,				A,
		SBB,            RN,				A,
		MPY,            RN,				A,
		CMP,            RN,				A,
		DAC,            RN,				A,
		DSB,            RN,				A,
// 20-2F
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		MOV,			BYTE,			A,
		AND,            BYTE,			A,
		OR,				BYTE,			A,
		XOR,            BYTE,			A,
		BTJO,           BYTE,			A,
		BTJZ,           BYTE,			A,
		ADD,            BYTE,			A,
		ADC,            BYTE,			A,
		SUB,            BYTE,			A,
		SBB,            BYTE,			A,
		MPY,            BYTE,			A,
		CMP,            BYTE,			A,
		DAC,            BYTE,			A,
		DSB,            BYTE,			A,
// 30-3F
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		MOV,			RN,				B,
		AND,            RN,				B,
		OR,				RN,				B,
		XOR,            RN,				B,
		BTJO,           RN,				B,
		BTJZ,           RN,				B,
		ADD,            RN,				B,
		ADC,            RN,				B,
		SUB,            RN,				B,
		SBB,            RN,				B,
		MPY,            RN,				B,
		CMP,            RN,				B,
		DAC,            RN,				B,
		DSB,            RN,				B,
// 40-4F
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		MOV,			RN,				RN,
		AND,            RN,				RN,
		OR,				RN,				RN,
		XOR,            RN,				RN,
		BTJO,           RN,				RN,
		BTJZ,           RN,				RN,
		ADD,            RN,				RN,
		ADC,            RN,				RN,
		SUB,            RN,				RN,
		SBB,            RN,				RN,
		MPY,            RN,				RN,
		CMP,            RN,				RN,
		DAC,            RN,				RN,
		DSB,            RN,				RN,
// 50-5F
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		MOV,			BYTE,			B,
		AND,            BYTE,			B,
		OR,				BYTE,			B,
		XOR,            BYTE,			B,
		BTJO,           BYTE,			B,
		BTJZ,           BYTE,			B,
		ADD,            BYTE,			B,
		ADC,            BYTE,			B,
		SUB,            BYTE,			B,
		SBB,            BYTE,			B,
		MPY,            BYTE,			B,
		CMP,            BYTE,			B,
		DAC,            BYTE,			B,
		DSB,            BYTE,			B,
// 60-6F
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		MOV,			B,				A,
		AND,            B,				A,
		OR,				B,				A,
		XOR,            B,				A,
		BTJO,           B,				A,
		BTJZ,           B,				A,
		ADD,            B,				A,
		ADC,            B,				A,
		SUB,            B,				A,
		SBB,            B,				A,
		MPY,            B,				A,
		CMP,            B,				A,
		DAC,            B,				A,
		DSB,            B,				A,
// 70-7F
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		MOV,			BYTE,			RN,
		AND,            BYTE,			RN,
		OR,				BYTE,			RN,
		XOR,            BYTE,			RN,
		BTJO,           BYTE,			RN,
		BTJZ,           BYTE,			RN,
		ADD,            BYTE,			RN,
		ADC,            BYTE,			RN,
		SUB,            BYTE,			RN,
		SBB,            BYTE,			RN,
		MPY,            BYTE,			RN,
		CMP,            BYTE,			RN,
		DAC,            BYTE,			RN,
		DSB,            BYTE,			RN,
// 80-8F
		MOVP,           PN,             A,
		DB,				OPCODE,         0,
		MOVP,			A,				PN,
		ANDP,			A,				PN,
		ORP,			A,				PN,
		XORP,			A,				PN,
		BTJOP,			A,				PN,
		BTJZP,			A,				PN,
		MOVD,			WORD,			RN,
		DB,				OPCODE,         0,
		LDA,            ADDR,			0,
		STA,            ADDR,			0,
		BR,            	ADDR,			0,
		CMPA,           ADDR,			0,
		CALL,           ADDR,			0,
		DB,				OPCODE,         0,
// 90-9F
		DB,				OPCODE,         0,
		MOVP,			PN,         	B,
		MOVP,			B,				PN,
		ANDP,			B,				PN,
		ORP,			B,				PN,
		XORP,			B,				PN,
		BTJOP,			B,				PN,
		BTJZP,			B,				PN,
		MOVD,			RN,				RN,
		DB,				OPCODE,         0,
		LDA,            ATRN,			0,
		STA,            ATRN,			0,
		BR,            	ATRN,			0,
		CMPA,           ATRN,			0,
		CALL,           ATRN,			0,
		DB,				OPCODE,         0,
// A0-AF
		DB,				OPCODE,         0,
		DB,				OPCODE,         0,
		MOVP,			BYTE,			PN,
		ANDP,			BYTE,			PN,
		ORP,			BYTE,			PN,
		XORP,			BYTE,			PN,
		BTJOP,			BYTE,			PN,
		BTJZP,			BYTE,			PN,
		MOVD,			WORD_B,			RN,
		DB,				OPCODE,         0,
		LDA,            ADDR_B,			0,
		STA,            ADDR_B,			0,
		BR,            	ADDR_B,			0,
		CMPA,           ADDR_B,			0,
		CALL,           ADDR_B,			0,
		DB,				OPCODE,         0,
// B0-BF
		TSTA,           0,				0,
		DB,				OPCODE,         0,
		DEC,            A,				0,
		INC,            A,				0,
		INV,           	A,				0,
		CLR,           	A,				0,
		XCHB,           A,				0,
		SWAP,           A,				0,
		PUSH,           A,				0,
		POP,           	A,				0,
		DJNZ,           A,				0,
		DECD,           A,				0,
		RR,           	A,				0,
		RRC,           	A,				0,
		RL,           	A,				0,
		RLC,           	A,				0,
// C0-CF
		MOV,			A,				B,
		TSTB,           0,              0,
		DEC,            B,				0,
		INC,            B,				0,
		INV,           	B,				0,
		CLR,           	B,				0,
		XCHB,           B,				0,
		SWAP,           B,				0,
		PUSH,           B,				0,
		POP,           	B,				0,
		DJNZ,           B,				0,
		DECD,           B,				0,
		RR,           	B,				0,
		RRC,           	B,				0,
		RL,           	B,				0,
		RLC,           	B,				0,
// D0-DF
		MOV,			A,				RN,
		MOV,			B,				RN,
		DEC,            RN,				0,
		INC,            RN,				0,
		INV,           	RN,				0,
		CLR,           	RN,				0,
		XCHB,           RN,				0,
		SWAP,           RN,				0,
		PUSH,           RN,				0,
		POP,           	RN,				0,
		DJNZ,           RN,				0,
		DECD,           RN,				0,
		RR,           	RN,				0,
		RRC,           	RN,				0,
		RL,           	RN,				0,
		RLC,           	RN,				0,
// E0-EF
		JMP,			OFST,			0,
		JN,           	OFST,			0,
		JZ,           	OFST,			0,
		JC,           	OFST,			0,
		JP,           	OFST,			0,
		JPZ,            OFST,			0,
		JNZ,            OFST,			0,
		JNC,            OFST,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
// F0-FF
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
		TRAP,			NTRAP,			0,
// END
        0,              0,              0
};

