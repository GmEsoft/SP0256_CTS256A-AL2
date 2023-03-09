/*
    SP0256A - Wave Writer.

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

#include <cstdio>

// .WAV file writer
class WaveWriter
{
public:
	WaveWriter() : file_( 0 ), lastSamples_( 0 ), errno_( 0 )
	{
	}


	WaveWriter( const char *filename, const unsigned waveFreq, const unsigned sampFreq, const unsigned nChannels, const unsigned nBitsPerSample );

	virtual ~WaveWriter()
	{
		close();
	}

	// Create a wave file
	int create( const char *filename, const unsigned waveFreq, const unsigned sampFreq, const unsigned nChannels, const unsigned nBitsPerSample );

	// Get the system errno of the last disk i/o
	int getErrno() const
	{
		return this->errno_;
	}

	// Write a number of samples
	void write( size_t nChans, int* samples );

	// Write a monophonic sample
	void write( int sample )
	{
		write( 1, &sample );
	}

	// Write a stereophonic sample
	void write( int sampleL, int sampleR )
	{
		int samples[] = { sampleL, sampleR };
		write( 2, samples );
	}

	// Close the .WAV file
	void close();

private:
	FILE *file_;
	int errno_;
	unsigned waveFreq_;
	unsigned sampFreq_;
	unsigned nChannels_;
	unsigned nBitsPerSample_;
	long riffLenOffset_, dataLenOffset_;
	long dataSize_;
	int *lastSamples_;
	unsigned long cnt_, lastcnt_;
};
