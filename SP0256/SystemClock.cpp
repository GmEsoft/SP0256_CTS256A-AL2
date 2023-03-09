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

#include "SystemClock.h"

#include <time.h>

#define _AUDIO_ENABLE_ 0
static const long AUTO_TURBO_MIN = 0x1000;
static const long AUTO_TURBO_MAX = 0x10000;

void SystemClock::resetAutoTurbo()
{
	if ( autoTurbo_ )
	{
		if ( autoCycles_ < AUTO_TURBO_MIN )
		{
			if ( turbo_ )
				turbo_ = false;
		}
	}
	autoCycles_ = 0;
}

void SystemClock::runCycles(long cycles)
{
	cycles *= cyclesMult_;
	cycles_ += cycles;
	autoCycles_ += cycles;

	if ( clockSpeed_ && !turbo_ )
	{
		if ( autoTurbo_ && autoCycles_ > AUTO_TURBO_MAX )
			turbo_ = true;

		while( cycles_ > 0 )
		{
			// Wait 1/100 sec clock change
			if ( clock100_ < 0 )
			{	// 1st call: initialize
				clock100_ = clock() / ( CLOCKS_PER_SEC / throttleFreq_ );
#if _AUDIO_ENABLE_
				cyclesAudio = 0;
#endif
				cycles_ = 0;
			}

			long s = clock100_;
			long timeout = 500L;

			while ( s == clock100_  )
			{
				s = clock() / ( CLOCKS_PER_SEC / throttleFreq_ );
				if ( s == clock100_ )
				{
					sleep( 5 );
					timeout -= 5;
					if ( timeout < 0 )
					{
						clock100_ = s = -1;
					}
				}
			}

			// ds = elapsed hundredths of seconds
			long ds = s - clock100_;
			clock100_ = s;

			while ( ds < 0 )
			{
				ds += 3600L*throttleFreq_; // Add 1 hour
			}

			if ( ds > 10L*throttleFreq_ )
			{
				// avoid lockups after wake-up
				clock100_ = -1;
			}
			else
			{

				cycles_ -= ds * 1000L * clockSpeed_ / throttleFreq_;
#if _AUDIO_ENABLE_
				cyclesAudio += ds * 1000L * clockspeed * ( AudioFreq/1000 ) / ThrottleFreq;
#endif
				// RTCINT counter
				rtcTest_ += ds * rtcRate_;

				// Update wave output
#if _AUDIO_ENABLE_
				outWaveUpdate();
#endif
				if ( rtcTest_ > 0 )
				{
					trigIrq();
					rtcTest_ -= throttleFreq_;
					//cprintf("<INT RT>");
				}
			}
		}
#if _AUDIO_ENABLE_
		outWaveUpdate();
#endif
	}
	else
	{
		clock100_ = -1;
		cycles_ = 0;
	}
}
