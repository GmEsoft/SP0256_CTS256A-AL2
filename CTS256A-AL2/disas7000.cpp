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

#define _CRT_SECURE_NO_WARNINGS 1

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys\stat.h>

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

// MCS-51 DISASSEMBLER-SIMULATOR ////////////////////////////////////////////////

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

//  Mnemonics for disassembler
char mnemo[][6] = {
	"ADC",	"ADD",	"AND",	"ANDP",	"BTJO",	"BTJOP","BTJZ",	"BTJZP",
	"BR",	"CALL",	"CLR",	"CLRC",	"CMP",	"CMPA",	"DAC",	"DEC",
	"DECD",	"DINT",	"DJNZ",	"DSB",	"EINT",	"IDLE",	"INC",	"INV",
	"JMP",	"JN",	"JZ",	"JC",	"JP",	"JPZ",	"JNZ",	"JNC",
	"LDA",	"LDSP",	"MOV",	"MOVD",	"MOVP",	"MPY",	"NOP",	"OR",
	"ORP",	"POP",	"PUSH",	"RETI",	"RETS",	"RL",	"RLC",	"RR",
	"RRC",	"SBB",	"SETC",	"STA",	"STSP",	"SUB",	"SWAP",	"TRAP",
	"TSTA",	"TSTB",	"XCHB",	"XOR",	"XORP",	"DB"
	};

//  Enumerated constants for operands, also array subscripts
enum {		// operand
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


char			noNewEqu = 0;
char			labelcolon = 0;

signed char		offset;
uint			pc;

int				pcOffset = 0;
ushort			pcOffsetBeg, pcOffsetEnd;
char			pcOffsetSeg = 'R';

// Symbols table
symbol_t		*tms7000Symbols = NULL;
uint			tms7000SymbolsSize = 0;
uint			nTms7000Symbols = 0;
uint			nNewTms7000Symbols = 0;

static char *comment;


void setTms7000Symbols( symbol_t *pSymbols, int pNSymbols, int pSymbolsSize )
{
	tms7000Symbols = pSymbols;
	nTms7000Symbols = pNSymbols;
	nNewTms7000Symbols = nTms7000Symbols;
	tms7000SymbolsSize = pSymbolsSize;
	qsort(tms7000Symbols, nTms7000Symbols, sizeof(symbol_t), (compfptr_t)symSort);
}

void updateTms7000Symbols()
{
	setTms7000Symbols( tms7000Symbols, nNewTms7000Symbols, tms7000SymbolsSize );
}

void resetTms7000Symbols()
{
}

// comparison function for qsort() and bsearch()
int  symSort(const void *_a, const void *_b)
{
	const symbol_t *a = (const symbol_t *)_a;
    const symbol_t *b = (const symbol_t *)_b;
    if (a->val < b->val) return -1;
    if (a->val > b->val) return 1;
    if (a->seg < b->seg) return -1;
    if (a->seg > b->seg) return 1;
    return 0;
}

int  symCompName(symbol_t *a, symbol_t *b)
{
    return strcmp (a->name, b->name);
}

static char getCodeSeg()
{
	return pcOffset ? pcOffsetSeg : 'C';
}


// get label of given code address
char* getLabel(uint val, char /*ds*/)
{
    static char name[40] ;

    symbol_t symtofind[1];
    symbol_t *sym;

	comment = NULL;

	name[0] = 0;

	symtofind->val = val;
    symtofind->seg = getCodeSeg();

    sym = (symbol_t*)bsearch(symtofind, tms7000Symbols, nTms7000Symbols, sizeof(symbol_t), (compfptr_t)symSort);
    if (sym == NULL)
	{
		return name;
	}

    strcpy (name, sym->name);
	if ( labelcolon )
		strcat (name, ":");

    return name;
}

// set label generated (DS labels)
void setLabelGen( uint val )
{
    symbol_t symtofind[1];
    symbol_t *sym;

	symtofind->val = val;
    symtofind->seg = getCodeSeg();

    sym = (symbol_t*)bsearch(symtofind, tms7000Symbols, nTms7000Symbols, sizeof(symbol_t), (compfptr_t)symSort);
}

// get label and offset of given code address
char* getLabelOffset(uint val)
{
    static char name[40] ;
	unsigned i;

    symbol_t symtofind[1];
    symbol_t *sym;

    symtofind->val = val;
    symtofind->seg = getCodeSeg();

    sym = tms7000Symbols;

	for ( i=0; i<nTms7000Symbols; ++i )
	{
		if ( symSort( symtofind, tms7000Symbols+i ) < 0 )
			break;
	}

    name[0] = 0;

	if (i>0)
	{
		if ( tms7000Symbols[i-1].val && val-tms7000Symbols[i-1].val < 0x0400 )
			sprintf( name, "%s+%Xh", tms7000Symbols[i-1].name, val-tms7000Symbols[i-1].val );
    }

    return name;
}

// Data Read Routine (memory address space)
uchar getData_null(ushort /*addr*/)
{
	return 0xFF;
}

readfptr_t vgetData = getData_null;

void setTms7000MemIO( readfptr_t getData )
{
	vgetData = getData;
}

#define getData (*vgetData)

//  get next instruction byte (sim)
#define fetch() getData( ushort( pc++ ) )

//  return hex-string or label for double-byte x (dasm)
char* getXAddr( uint x )
{
	static char addr[41];
	symbol_t symtofind;
	symbol_t *sym;

	comment = NULL;

	symtofind.val = x;
	symtofind.seg = getCodeSeg();

	sym = (symbol_t*)bsearch(&symtofind, tms7000Symbols, nTms7000Symbols, sizeof(symbol_t), (compfptr_t)symSort);

	if ( sym )
	{
		strcpy(addr, sym->name);
	}
	else
	{
		uint xorg = x;
		if ( pcOffsetSeg != 'C' )
			xorg -= pcOffset;

		sprintf( addr, ">%04X", xorg );
	}
	return addr;
}

// get comment associated to label of given code address from last getXAddr()/getLabel() call
char* getLastComment()
{
	return comment;
}

//	return internal byte address as label or as hex string
char* getDataAddr ()
{
	static char addr[41];
	unsigned x;
	symbol_t symtofind;
	symbol_t *sym;

    addr[0] = 0;
	x = fetch();

	symtofind.val = x;
	symtofind.seg = 'D';

	sym = (symbol_t*)bsearch( &symtofind, tms7000Symbols, nTms7000Symbols, sizeof( symbol_t ), (compfptr_t)symSort );

	if (sym != NULL && sym->name[0] != 0)
	{
		strcpy(addr, sym->name);
		return addr;
	}

	sprintf (addr, ">%02X", x);

	return addr;
}


// fetch long external address and return it as hex string or as label
char* getLAddr()
{
	uint x;
	char oldseg = pcOffsetSeg;
	char *ret;

	x = fetch () << 8;
	x += fetch ();
	if ( pcOffset && x + pcOffset >= pcOffsetBeg && x + pcOffset < pcOffsetEnd )
		x += pcOffset;
	else
		pcOffsetSeg = 'C';
	ret = getXAddr( x );
	pcOffsetSeg = oldseg;
	return ret;
}

// fetch short relative external address and return it as hex string or as label
char* getSAddr()
{
	uint x;
	signed char d;
	d = (signed char) fetch ();
	x = pc + d;
	return getXAddr( x );
}

// return operand name or value
char* getOperand(int opcode, int opn)
{
	static char op[41];
	unsigned x;

    strcpy (op, "??");

    switch (opn)
	{
	case 0:
		return NULL;
    case A:
		return "A";
	case B:
		return "B";
	case RN:
		x = fetch();
		sprintf( op, "R%d", x );
		break;
	case PN:
		x = fetch();
		sprintf( op, "P%d", x );
		break;
	case BYTE:
		x = fetch();
		sprintf( op, "%%>%02X", x );
		break;
	case WORD:
		strcpy( op, "%" );
		strcat( op, getLAddr() );
		break;
	case WORD_B:
		strcpy( op, "%" );
		strcat( op, getLAddr() );
		strcat( op, "(B)" );
		break;
	case OFST:
		return getSAddr();
	case ADDR:
		strcpy( op, "@" );
		strcat( op, getLAddr() );
		break;
	case ADDR_B:
		strcpy( op, "@" );
		strcat( op, getLAddr() );
		strcat( op, "(B)" );
		break;
	case ATRN:
		x = fetch();
		sprintf( op, "*R%d", x );
		break;
	case ST:
		return "ST";
	case NTRAP:
		sprintf( op, "%d", 255-opcode );
		break;
	case OPCODE:
		sprintf( op, ">%02X", opcode );
		break;
	}
	return op;
}

// get 1st operand name or value
char* getOperand1(int opcode)
{
	return getOperand(opcode, instrTable[opcode].opn1);
}

// get 2nd operand name or value
char* getOperand2(int opcode)
{
	return getOperand(opcode, instrTable[opcode].opn2);
}

// add comment if any
void addComment( char *src, int size, char *pComment )
{
	size_t n;

	if ( pComment )
	{
		for ( n = strlen( src ); n < 24; ++n )
		{
			src[n] = ' ';
		}
		src[n] = 0;
		strncat_s( src, size, pComment, size - n - 1 );
	}
}

// get single instruction source
char* source ()
{
	ushort opcode;
	static char src[80];
	size_t i;
	char* op;

	opcode = fetch ();

	strcpy (src, mnemo[instrTable[opcode].mnemon]);

	for (i=strlen(src);i<8;i++)
		src[i] = ' ';

	src[i] = '\0';

	comment = 0;

	op = getOperand1( opcode );
	if ( op )
	{
		strcat( src, op );
		op = getOperand2( opcode );
		if ( op )
		{
			strcat( src, "," );
			strcat( src, op );
			switch( instrTable[opcode].mnemon )
			{
			case BTJO:
			case BTJOP:
			case BTJZ:
			case BTJZP:
				strcat( src, "," );
				strcat( src, getSAddr() );
			}
		}
	}

	addComment( src, sizeof(src), comment );

	for (i=strlen(src);i<48;i++) {
		src[i] = ' ';
	}

	src[i] = '\0';

	return src;
}


// PROCESSOR INSTRUCTIONS TABLE ///////////////////////////////////////////////

//  Processor's instruction set
instr_t instrTable[] = {
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

