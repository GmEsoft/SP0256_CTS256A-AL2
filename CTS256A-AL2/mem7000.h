#ifndef __MEM51_H__
#define __MEM51_H__

#define SYMSIZE 10240        // symbol table size


typedef struct
{
	unsigned int beg, end;
} range_t;

extern range_t ranges[];
extern unsigned nranges;

// Data Write Routine (memory address space)
unsigned char putdata( unsigned short addr, unsigned char byte );

// Data Read Routine (memory address space)
unsigned char getdata( unsigned short addr );


#endif
