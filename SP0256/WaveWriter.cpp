#include "WaveWriter.h"

#include "types.h"

#include <cstdlib>


static int fput32( int n, FILE *out )
{
	int i;
	for ( i=0; i<4; ++i ) {
		fputc( n & 0xFF, out );
		n >>= 8;
	}
	return n;
}

static int fput16( int n, FILE *out )
{
	int i;
	for ( i=0; i<2; ++i ) {
		fputc( n & 0xFF, out );
		n >>= 8;
	}
	return n;
}

static int fput8( int n, FILE *out )
{
	fputc( n & 0xFF, out );
	return n;
}

WaveWriter::WaveWriter( const char *filename, const unsigned freq, const unsigned nChannels, const unsigned nBitsPerSample )
: freq_( freq ), nChannels_( nChannels ), nBitsPerSample_( nBitsPerSample )
{
	create( filename, freq, nChannels, nBitsPerSample );
}

int WaveWriter::create( const char *filename, const unsigned freq, const unsigned nChannels, const unsigned nBitsPerSample )
{
	this->errno_ = fopen_s( &file_, filename, "wb" );
	if ( this->errno_ )
		return this->errno_;

	freq_ = freq;
	nChannels_ = nChannels;
	nBitsPerSample_ = nBitsPerSample;
	dataSize_ = 0;

	// RIFF Header
	fputs( "RIFF", file_ );
	riffLenOffset_ = ftell( file_ );
	fput32( 36, file_ );
	fputs( "WAVE", file_ );

	// Subchunk "fmt "
	fputs( "fmt ", file_ ); // Subchunk1ID
	fput32( 16, file_ ); // Subchunk1size
	fput16( 1, file_ ); // AudioFormat
	fput16( nChannels, file_ ); // NumChannels
	fput32( freq, file_ ); // SampleRate
	fput32( freq * nChannels * nBitsPerSample / 8, file_ ); // ByteRate
	fput16( nChannels * nBitsPerSample / 8, file_ ); // BlockAlign
	fput16( nBitsPerSample, file_ ); // BitsPerSample

	// Subchunk "data"
	fputs( "data", file_ ); // Subchunk2ID
	dataLenOffset_ = ftell( file_ );
	fput32( 0, file_ ); // Subchunk2size
	return 0;
}

void WaveWriter::close()
{
	if ( file_ )
	{
		fseek( file_, riffLenOffset_, SEEK_SET );
		fput32( dataSize_ + 36, file_ );
		fseek( file_, dataLenOffset_, SEEK_SET );
		fput32( dataSize_, file_ );
		fclose( file_ );
		file_ = 0;

	}
}

void WaveWriter::write( size_t nChans, int *samples )
{
	if ( file_ )
	{
		size_t nSample = 0;
		for ( size_t i=0; i<nChannels_; ++i )
		{
			switch ( nBitsPerSample_ )
			{
			case 8:
				fput8( samples[nSample], file_ );
				++dataSize_;
				break;
			case 16:
				fput16( samples[nSample], file_ );
				dataSize_ += 2;
				break;
			case 32:
				fput32( samples[nSample], file_ );
				dataSize_ += 4;
				break;
			}
			if ( ++nSample == nChans )
				nSample = 0;
		}
	}
}
