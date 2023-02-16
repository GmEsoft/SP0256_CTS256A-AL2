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