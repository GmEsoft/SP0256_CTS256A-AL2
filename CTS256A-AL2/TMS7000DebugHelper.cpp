#pragma warning(disable:4996)	// warning C4996: '%0': This function or variable may be unsafe. 

#include "TMS7000DebugHelper.h"

#include "disas7000.h"

#include <cstring>

void TMS7000DebugHelper::printRegNamesLine( Console_I &console ) 
{
	console.printf( "A  B  Sp St CNZI3210 R2R3 R4R5 R6R7 R8R9" );
}

void TMS7000DebugHelper::printRegsLine( Console_I &console )
{
	char intStatus = flags_.i
		? ( cpu_.getIRQ() ? '*' : '+' )		// Interrupt enabled:  '*' = int. pending; '+' = no int. pending
		: ( cpu_.getIRQ() ? '=' : '-' );	// Interrupt disabled: '=' = int. pending; '-' = no int. pending
	//              A     B    SP   ST  I C N Z I 3 2 1 0  R2  R3   R4  R5   R6  R7   R8  R9
	console.printf( "%02X %02X %02X %02X%c%d%d%d%d%d%d%d%d %02X%02X %02X%02X %02X%02X %02X%02X",
		data_[0], data_[1], sp_, (uchar&)flags_,
		intStatus, flags_.c&1, flags_.n&1, flags_.z&1, flags_.i&1, flags_.b3&1, flags_.b2&1, flags_.b1&1, flags_.b0&1,
		data_[2], data_[3], data_[4], data_[5], data_[6], data_[7], data_[8], data_[9]
		);
}

void TMS7000DebugHelper::printSource( Console_I &console, uint &pc )
{
	uint nextpc = pc;
	const char *src = getSource( nextpc );

	console.printf( "%04X ", pc );
	int i = 0;
	for ( ; pc < nextpc; ++pc, ++i )
		console.printf( "%02X", getSourceByte( pc ) );
	for ( ; i < 4; ++i )
		console.printf( "  " );
	console.printf( " %.22s", src );
}

void TMS7000DebugHelper::printTabSourceWidth( Console_I &console )
{
	console.printf( "%*c", 36, ' ' );
}

static CPU *s_pCpu = 0;

static uchar s_getData( ushort addr )
{
	if ( s_pCpu )
		return s_pCpu->getdata( addr );
	return 0xFF;
}

const char *TMS7000DebugHelper::getSource( uint &pc )
{
	static char src[80];
	s_pCpu = &cpu_;
	setTms7000MemIO( s_getData );
	disass_.setPC( pc );
	strcpy( src, disass_.source() );
	pc = disass_.getPC();
	for ( size_t i=6; i<strlen( src )-2; ++i )
		src[i] = src[i+2];
	return src;
}

uchar TMS7000DebugHelper::getSourceByte( uint pc )
{
	return cpu_.getdata( pc );
}

const char *TMS7000DebugHelper::getPointer( int n, uint &ptr )
{
	ptr = 0;
	return 0;
}

bool TMS7000DebugHelper::isCall( uint pc )
{
	uchar x = cpu_.getcode( pc );
	return	(	x == 0x12							// LCALL nnnn
			||	( x & 0x1F ) == 0x11				// ACALL nnnn
			);
}

bool TMS7000DebugHelper::isRet( uint pc )
{
	uchar x = cpu_.getcode( pc );
	return	(	x == 0x22							// RET
			||	x == 0x32							// RETI
			);
}

bool TMS7000DebugHelper::isSetSP( uint pc )
{
	return false;
}

bool TMS7000DebugHelper::isBreak( uint pc )
{
	return false;
}

bool TMS7000DebugHelper::isBreakRet( uint lastpc, uint retsp )
{
	return false;
}

