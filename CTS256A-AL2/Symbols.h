#pragma once

// SYMBOLS TABLE

#include "runtime.h"

struct symbol_t 
{
	char    name[41];
    uint    val;
    char    seg;
    uint  	call;
};

class Symbols
{
public:
	// Attach Z80 to external symbol table
	void setSymbols( symbol_t *pSymbols, int pNSymbols )
	{
		symbols_ = pSymbols;
		nSymbols_ = pNSymbols;
	}

	// Sort symbols
	static int  compVal(symbol_t *a, symbol_t *b);

	// Compare symbols by name
	static int  compName(symbol_t *a, symbol_t *b);

	// get symbol
	symbol_t* getSymbol( char seg, uint val );

	// get label of given code address
	char* getLabel(uint val);

	// get label and offset of given code address
	char* getLabelOffset(uint val);

private:
	symbol_t	*symbols_;
	int			nSymbols_;
};
