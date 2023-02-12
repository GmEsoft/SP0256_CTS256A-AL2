#pragma warning(disable:4996)	// warning C4996: '%0': This function or variable may be unsafe. 

#include "Symbols.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


typedef int (*compfptr_t)(const void*, const void*);

// Sort symbols
int  Symbols::compVal(symbol_t *a, symbol_t *b)
{
    if (a->val < b->val) return -1;
    if (a->val > b->val) return 1;
    if (a->seg < b->seg) return -1;
    if (a->seg > b->seg) return 1;
	return 0;
}

// Compare symbols by name
int  Symbols::compName(symbol_t *a, symbol_t *b)
{
    return strcmp(a->name, b->name);
}

// get symbol
symbol_t* Symbols::getSymbol( char seg, uint val )
{
    symbol_t symtofind;
    symtofind.val = val;
    symtofind.seg = seg;
	return (symbol_t*)bsearch( &symtofind, symbols_, nSymbols_, sizeof(symbol_t), (compfptr_t)compVal );
}

// get label of given code address
char* Symbols::getLabel(uint val)
{
    static char name[40];

	symbol_t *sym = getSymbol( 'C', val );
    if (sym != NULL){
        strcpy (name, sym->name);
        strcat (name, ":");
    } else {
        name[0] = 0; // static
    }

    return name;
}

// get label and offset of given code address
char* Symbols::getLabelOffset(uint val)
{
    static char name[40] ;
	int i;

	symbol_t symtofind;

    symtofind.val = val;
    symtofind.seg = 'C';

	for ( i = 0; i < nSymbols_; ++i )
	{
		if ( compVal( &symtofind, symbols_ + i ) < 0 )
			break;
	}

    name[0] = 0; // static

	if ( i > 0 )
	{
		if ( symbols_[i-1].val && ( val - symbols_[i-1].val < 0x0400 ) )
			sprintf( name, "%s + %Xh", symbols_[i-1].name, val - symbols_[i-1].val );
    }

    return name;
}
