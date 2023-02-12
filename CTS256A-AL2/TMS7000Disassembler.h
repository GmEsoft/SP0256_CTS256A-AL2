#pragma once

#include "disassembler.h"

class TMS7000Disassembler :
	public Disassembler
{
public:
	TMS7000Disassembler(void)
	{
	}

	~TMS7000Disassembler(void)
	{
	}

	//  return hex-string or label for double-byte x(dasm)
	const char *getxaddr(uint x);

	//	return internal byte address as label or as hex string
	const char *getdataaddr();

	//  return internal bit address as BIT name or as hex string
	const char *getbitaddr();

	// fetch long external address and return it as hex string or as label
	const char *getladdr();

	// fetch absolute segment external address and return it as hex string or as label
	const char *getaaddr(char opcode);

	// fetch short relative external address and return it as hex string or as label
	const char *getsaddr();

	// return operand name or value
	const char *getoperand(int opn);

	// get 1st operand name or value
	const char *getoperand1(int opcode);

	// get 2nd operand name or value
	const char *getoperand2(int opcode);

	// get single instruction source
	virtual const char *source();
};

