/*
    SP0256A - System Clock API.

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

// CLOCK API

class Clock_I
{
public:
	// do CPU cycles
	virtual void runCycles( long cycles ) = 0;

	// Set clock speed (kHz)
	virtual void setClockSpeed( long speed ) = 0;

	// Set RTC rate (kHz)
	virtual void setRtcRate( long rate ) = 0;

	// Set auto turbo
	virtual void setAutoTurbo( bool autoTurbo ) = 0;

	// destructor
	virtual ~Clock_I()
	{
	}
};
