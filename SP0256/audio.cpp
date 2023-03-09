/*
    SP0256A - Audio Interface.

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

#include "audio.h"

#include "Win32Audio.h"
#include "types.h"

#include "cstdio"

#define USE_AUDIO 1

// audio parameters ///////////////////////////////////////////////////////////
#define AUDIO_FILTER		96			// between 1 (strong) and 256 (none)
#define AUDIO_FREQ			20000		// Audio sample freq in Hz
#define AUDIO_BUFFER_FREQ	10			// Buffer switching frequency
#define AUDIO_NUM_BUFFERS	50			// Number of Buffers
#define AUDIO_BUFFER_LENGTH (2*AUDIO_FREQ/AUDIO_BUFFER_FREQ) // 2* because stereo
///////////////////////////////////////////////////////////////////////////////

static uchar	audioBuffers_[AUDIO_NUM_BUFFERS][AUDIO_BUFFER_LENGTH]; // Audio Output Buffers
static long		audioCurrentPos_ = 0;
static unsigned audioCurrentBuffer_ = 0;
static unsigned audioCurrentBufferPos_ = 0;
static uchar	audioCurrentLevelL_ = 0x80;
static uchar	audioCurrentLevelR_ = 0x80;
static long		cyclesAudio_ = 0;
static long		lastCyclesAudio_ = 0;
static long		cycles_ = 0;
static long		clockspeed_ = 10000;		// Clock speed in Hz

void outWaveUpdate()
{
	long nSamples;

	static uchar audioLastLevelL = 0x80;
	static uchar audioLastLevelR = 0x80;

#ifdef AUDIO_FILTER
	static uchar levelL = 0x80;
	static uchar levelR = 0x80;
#endif

	if ( cyclesAudio_ > 0 )
	{
		long curCyclesAudio = cyclesAudio_;
		long incCyclesAudio = cyclesAudio_ - lastCyclesAudio_;
		nSamples = cyclesAudio_ / clockspeed_;
		cyclesAudio_ -= nSamples * clockspeed_;

		long nCycles = -lastCyclesAudio_;

		while ( nSamples > 0 )
		{
			nCycles += clockspeed_;
			uchar iLevelL = uchar( audioLastLevelL + nCycles * ( audioCurrentLevelL_ - audioLastLevelL )  / incCyclesAudio );
			uchar iLevelR = uchar( audioLastLevelR + nCycles * ( audioCurrentLevelR_ - audioLastLevelR )  / incCyclesAudio );

#ifdef AUDIO_FILTER
			levelL += ( ( (unsigned)( iLevelL - levelL ) * AUDIO_FILTER ) >> 8 );
			levelR += ( ( (unsigned)( iLevelR - levelR ) * AUDIO_FILTER ) >> 8 );
			audioBuffers_[audioCurrentBuffer_][audioCurrentBufferPos_++] = levelL;
			audioBuffers_[audioCurrentBuffer_][audioCurrentBufferPos_++] = levelR;
#else
			audioBuffers_[audioCurrentBuffer_][audioCurrentBufferPos_++] = iLevelL;
			audioBuffers_[audioCurrentBuffer_][audioCurrentBufferPos_++] = iLevelR;
#endif
			if ( audioCurrentBufferPos_ == AUDIO_BUFFER_LENGTH )
			{
#if USE_AUDIO
				queueWaveBuffer( audioBuffers_[audioCurrentBuffer_], AUDIO_BUFFER_LENGTH );
#endif
				++audioCurrentBuffer_;
				audioCurrentBufferPos_ = 0;

				if ( audioCurrentBuffer_ == AUDIO_NUM_BUFFERS )
				{
					audioCurrentBuffer_ = 0;
				}
			}
			--nSamples;
		}
		lastCyclesAudio_ = cyclesAudio_;
	}
	audioLastLevelL = audioCurrentLevelL_;
	audioLastLevelR = audioCurrentLevelR_;
}

void outWave( uchar levelL, uchar levelR )
{
	outWaveUpdate();
	audioCurrentLevelL_ = levelL;
	audioCurrentLevelR_ = levelR;
}

void outWaveReset()
{
	audioCurrentLevelL_ = 0x80;
	audioCurrentLevelR_ = 0x80;
	cyclesAudio_ = 0;
}

void outWaveInit()
{
	initWaveOutDevice( AUDIO_FREQ );
}

void outWaveCycles( ulong cycles_ )
{
	cyclesAudio_ += cycles_ * AUDIO_FREQ;
}

void outWaveSetClockSpeed( long speed )
{
	clockspeed_ = speed;
}

void outWaveFlush()
{
	while( audioCurrentBufferPos_ < AUDIO_BUFFER_LENGTH )
	{
		audioBuffers_[audioCurrentBuffer_][audioCurrentBufferPos_++] = audioCurrentLevelL_;
		audioBuffers_[audioCurrentBuffer_][audioCurrentBufferPos_++] = audioCurrentLevelR_;
	}

	queueWaveBuffer( audioBuffers_[audioCurrentBuffer_], AUDIO_BUFFER_LENGTH );

	closeWaveOutDevice();

}
