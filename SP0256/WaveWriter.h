#pragma once

#include <cstdio>

class WaveWriter
{
public:
	WaveWriter() : file_( 0 )
	{
	}


	WaveWriter( const char *filename, const unsigned freq, const unsigned nChannels, const unsigned nBitsPerSample );

	virtual ~WaveWriter()
	{
		close();
	}

	int create( const char *filename, const unsigned freq, const unsigned nChannels, const unsigned nBitsPerSample );

	int getErrno() const
	{
		return this->errno_;
	}

	void write( size_t nChans, int* samples );

	void write( int sample )
	{
		write( 1, &sample );
	}

	void write( int sampleL, int sampleR )
	{
		int samples[] = { sampleL, sampleR };
		write( 2, samples );
	}

	void close();

private:
	FILE *file_;
	int errno_;
	unsigned freq_;
	unsigned nChannels_;
	unsigned nBitsPerSample_;
	long riffLenOffset_, dataLenOffset_;
	long dataSize_;
};