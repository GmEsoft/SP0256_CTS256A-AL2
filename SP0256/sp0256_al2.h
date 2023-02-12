#pragma once

#include "types.h"

namespace sp0256_al2
{

	extern const char * labels[];

	extern unsigned nlabels;

	extern const uint8_t mask[];

	enum allo
	{
		STOP = -1,
		PA1,	PA2,	PA3,	PA4,	PA5,	OY,		AY,		EH,
		KK3,	PP,		JH,		NN1,	IH,		TT2,	RR1,	AX,
		MM,		TT1,	DH1,	IY,		EY,		DD1,	UW1,	AO,
		AA,		YY2,	AE,		HH1,	BB1,	TH,		UH,		UW2,
		AW,		DD2,	GG3,	VV,		GG1,	SH,		ZH,		RR2,
		FF,		KK2,	KK1,	ZZ,		NG,		LL,		WW,		XR,
		WH,		YY1,	CH,		ER1,	ER2,	OW,		DH2,	SS,
		NN2,	HH2,	OR,		AR,		YR,		GG2,	EL,		BB2,
		END
	};

}
