//#define NDEBUG
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "mem7000.h"
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




FILE *readfile(const char *prompt, const char *mode);

char data[0x10000];

range_t ranges[1000];
unsigned nranges = 0;

// Data Write Routine (memory address space)
unsigned char putdata(unsigned short addr, unsigned char byte)
{
	// M1 System RAM
	data[addr] = byte;

	if ( !nranges || ranges[nranges-1].end != addr )
	{
		ranges[nranges].beg = addr;
		++nranges;
	}
	ranges[nranges-1].end = addr + 1;

    return byte;
}

// TRS-80 Data Read Routine (memory address space)
unsigned char getdata(unsigned short addr)
{
	unsigned char byte;

    byte = data[addr];
    return (unsigned char)byte;
}

