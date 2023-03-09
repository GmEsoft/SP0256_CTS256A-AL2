/*
    SP0256A - SP0256-AL2 ROM Data.

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
