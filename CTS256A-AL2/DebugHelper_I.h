#pragma once

#include "runtime.h"
#include "Memory_I.h"
#include "Console_I.h"

class DebugHelper_I
{
public:
	DebugHelper_I(void);
	virtual ~DebugHelper_I(void);
	virtual uint getPC() = 0;
	virtual uint getSP() = 0;
	virtual void sim() = 0;
	virtual Memory_I &getData() = 0;
	virtual void printRegNamesLine( Console_I &console ) = 0;
	virtual void printRegsLine( Console_I &console ) = 0;
	virtual void printSource( Console_I &console, uint &pc ) = 0;
	virtual void printTabSourceWidth( Console_I &console ) = 0;
	virtual const char *getSource( uint &pc ) = 0;
	virtual uchar getSourceByte( uint pc ) = 0;
	virtual const char *getPointer( int n, uint &ptr ) = 0;
	virtual bool isCall( uint pc ) = 0;
	virtual bool isRet( uint pc ) = 0;
	virtual bool isSetSP( uint pc ) = 0;
	virtual bool isBreak( uint pc ) = 0;
	virtual bool isBreakRet( uint lastpc, uint retsp ) = 0;

};

