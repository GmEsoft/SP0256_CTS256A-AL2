/*
    SP0256A - System Clock.

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

#include "Clock_I.h"

#include "Sleeper_I.h"
#include "IRQ_I.h"
#include "types.h"

class SystemClock :
	public Clock_I
{
public:
	SystemClock(void) :
		clockSpeed_( 0 ), clock100_( -1L ), rtcRate_( 30 ), rtcTest_( 0 ),
		throttleFreq_( 100 ), cycles_( 0 ), sleeper_( 0 ), irq_( 0 ), cyclesMult_( 1 ),
		autoCycles_( 0 ), autoTurbo_( false ), turbo_( false )
	{
	}

	virtual ~SystemClock(void)
	{
	}

	// do CPU cycles
	virtual void runCycles( long cycles );

	// Set cycles multiplier
	virtual void setCyclesMult( long cyclesMult )
	{
		cyclesMult_ = cyclesMult;
	}

	// Set clock speed (kHz)
	virtual void setClockSpeed( long speed )
	{
		clockSpeed_ = speed;
		cycles_ = 0;
	}

	// Set RTC rate (Hz)
	virtual void setRtcRate( long rate )
	{
		rtcRate_ = rate;
	}

	// Set auto turbo
	void setAutoTurbo( bool autoTurbo )
	{
		autoTurbo_ = autoTurbo;
	}

	// Reset auto turbo cycles counter
	void resetAutoTurbo();

	// Set sleeper
	void setSleeper( Sleeper_I *sleeper )
	{
		sleeper_ = sleeper;
	}

	// Set IRQ handler
	void setIRQ( IRQ_I *irq )
	{
		irq_ = irq;
	}

	void sleep( ulong millis )
	{
		if ( sleeper_ )
			sleeper_->sleep( millis );
	}

	void trigIrq()
	{
		if ( irq_ )
			irq_->trigger();
	}


protected:
	long			clockSpeed_;		// Clock speed in kHz
	long			clock100_;			// Clock as n/100 sec from midnight
	long			rtcRate_;			// RTC rate in Hz
	long			rtcTest_;			//
	long			throttleFreq_;		//
	long			cycles_;			// cycles to consume
	long			cyclesMult_;		// cycles multiplier
	long			autoCycles_;		// auto cycles counter
	bool			autoTurbo_;			// enable auto turbo
	bool			turbo_;				// turbo is active
	Sleeper_I		*sleeper_;			// sleep( millis )
	IRQ_I			*irq_;				// IRQ handler
};
