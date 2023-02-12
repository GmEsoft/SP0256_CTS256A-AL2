#pragma warning(disable:4996)	// warning C4996: '%0': This function or variable may be unsafe. 

#include "TMS7000Disassembler.h"
#include "disas7000.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <io.h>
#include <search.h>



//  return hex-string or label for double-byte x( dasm )
const char *TMS7000Disassembler::getxaddr( uint x )
{
	return getXAddr( x );
}


//	return internal byte address as label or as hex string
const char *TMS7000Disassembler::getdataaddr()
{
	return "";
}

//  return internal bit address as BIT name or as hex string
const char *TMS7000Disassembler::getbitaddr()
{
	return "";
}

// fetch long external address and return it as hex string or as label
const char *TMS7000Disassembler::getladdr()
{
	return getLAddr();
}

// fetch absolute segment external address and return it as hex string or as label
const char *TMS7000Disassembler::getaaddr( char opcode )
{
	return "";
}

// fetch short relative external address and return it as hex string or as label
const char *TMS7000Disassembler::getsaddr()
{
	return "";//getSAddr();
}

// return operand name or value
const char *TMS7000Disassembler::getoperand( int opn )
{
	return "";//getOperand();
}

// get 1st operand name or value
const char *TMS7000Disassembler::getoperand1( int opcode )
{
	return "";//getOperand();
}

// get 2nd operand name or value
const char *TMS7000Disassembler::getoperand2( int opcode )
{
	return "";//getOperand();
}


// get single instruction source
const char *TMS7000Disassembler::source()
{
	::pc = pc_;
	const char *s = ::source();
	pc_ =::pc;
	return s;
}

