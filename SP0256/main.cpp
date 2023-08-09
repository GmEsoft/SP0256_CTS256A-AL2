/*
    SP0256 - Main module.

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


#include "stdafx.h"

#define NAME	"SP0256(tm) Emulator"
#define VERSION	"v0.0.4-alpha"


#include "Win32Sleeper.h"
#include "SystemClock.h"
#include "audio.h"
#include "WaveWriter.h"

#include "sp0256.h"

#include "sp0256_al2.h"	// SP0256-AL2 "Narrator"
#include "sp0256_012.h"	// SP0256-012 "Intellivoice"

#include <map>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>


using namespace sp0256_al2;

typedef std::map< std::string, size_t > dict_t;

enum model_t
{
	_012, _AL2
};

static void help()
{
	puts(
		NAME " - " VERSION "\n\n"
		"GI/Microchip SP0256-AL2 Narrator(tm) and SP0256-012 Intellivoice(tm) Speech Processor\n\n"
		"Usage:\n"
		"sp0256 [-m{AL2|012}] [-e] [-v] [-xClockFreq] [ -t | -b | -a ] [ -i{inFile|-} ] [-wWavFile]\n"
		"-mAL2     Select Narrator(tm) speech ROM\n"
		"-m012     Select Intellivoice speech ROM\n"
		"-e        Echo speech elements (words or allophones)\n"
		"-v        Verbose mode\n"
		"-d[D|S|T] Set debug for [D]ebug, [S]amples or [T]race\n"
		"-xClkFreq Xtal Clock Frequency in Hz (range: 1000000..5000000)\n"
		"-iInFile  Say File\n"
		"-i-       Say from stdin: echo ... | sp0256 -i-\n"
		"-t        Text Mode (labels) (default)\n"
		"-b        Binary Mode (addresses)\n"
		"-a        Pronounce all words or allophones in speech ROM\n"
		"-wWavFile Create .wav file\n"
	);
}

// SP0256-AL2 "Narrator" Sample Speech
// "SP0256-AL2 Narrator"
static int codes_al2[] = { 
	EH, EH, SS, SS, PA2,
	PP, IY, PA2,
	ZZ, YR, OW, PA2,
	TT2, UW2, PA2,
	FF, FF, AY, VV, PA2,
	SS, SS, IH, IH, PA3, KK2, SS, PA2,
	DD2, AE, SH, PA2,
	EY, PA2,
	EH, EH, EL, PA2,
	TT2, UW2, PA5,
	NN2, AE, RR2, AE, PA3, TT2, ER1, PA3,
	STOP };

// SP0256-012 "Intellivoice" Sample Speech
// "Mattel Electronics Presents - Zero Two Five Six - And - Zero One Two"
static int codes_012[] = { 6, 2, 7, 9, 12, 13, 2, 42, 2, 7, 8, 9, -1 };


static std::string toUpperCase( const std::string &str )
{
	std::string uStr = str;
	std::transform( uStr.begin(), uStr.end(), uStr.begin(), ::toupper );
	return uStr;
}


int _tmain(int argc, _TCHAR* argv[])
{
	model_t model = _AL2;
	char mode = 0;
	char verbose = 0;
	char echo = 0;
	char eos = 0;
	int debug = 0;
	dict_t dict;
	int xtal = 3120000;
	int waveFreq = 0;

	const char* waveFileName = 0;

	int errno_ = 0;
	const char *fileName = 0;

	std::istream *pistr = &std::cin;
	std::stringstream sstr;
	std::auto_ptr< std::fstream > pfstr;

	for ( int i=1; i<argc; ++i )
	{
		char *s = argv[i];

		if ( *s == '-' )
		{
			++s;
			switch ( toupper( *s ) )
			{
			case 'M':
				++s;
				if ( *s == ':' )
					++s;
				if ( !_strcmpi( s, "012" ) ) 
				{
					model = _012;
				}
				else if ( !_strcmpi( s, "AL2" ) )
				{
					model = _AL2;
				}
				else
				{
					puts( NAME " - " VERSION );
					printf( "Unknown Model: %s\n", s );
					return 1;
				}
				break;
			case 'B': // Bin file
				mode = 'B';
				break;
			case 'T': // Text file (default)
				mode = 'T';
				break;
			case 'I': // Input File
				++s;
				if ( *s == ':' )
					++s;
				if ( !_strcmpi( s, "-" ) )
					pistr = &std::cin;
				else
				{
					pfstr.reset( new std::fstream( fileName = s, std::fstream::in ) );
					pistr = pfstr.get();
					if ( !pfstr->is_open() )
					{
						printf( "Failed to open %s\n", s );
						errno_ = errno;
					}
				}
				if ( !mode )
					mode = 'T';
				break;
			case 'W': // Output to .WAV file
				++s;
				if ( isdigit( *s ) )
					sscanf_s( s, "%d", &waveFreq );
				while ( isdigit( *s ) )
					++s;
				if ( *s == ':' )
					++s;
				waveFileName = s;
				break;
			case 'A': // All Sounds/Allophones
				mode = 'A';
				break;
			case 'E': // Echo
				echo = 1;
				break;
			case 'D': // Debug ('D'ebug, 'S'amples)
				debug = 0;
				++s;
				if ( *s == ':' )
					++s;
				while ( *s )
				{
					if ( toupper( *s ) == 'T' ) // Trace
						debug |= 1;
					else if ( toupper( *s ) == 'S' ) // Samples
						debug |= 2;
					else if ( toupper( *s ) == 'D' ) // Debug (single step)
						debug |= 5;
					++s;
				}
				if ( !debug )
					debug = 0xFFFF;
				sp0256_setDebug( debug );
				break;
			case 'V': // Verbose
				verbose = 1;
				break;
			case 'X': // Xtal
				++s;
				if ( *s == ':' )
					++s;
				sscanf_s( s, "%d", &xtal );
				if ( xtal < 1000000 )
				{
					puts( NAME " - " VERSION );
					printf( "XTAL %d < 1000000", xtal );
					return 1;
				}
				if ( xtal > 5000000 )
				{
					puts( NAME " - " VERSION );
					printf( "XTAL %d > 5000000", xtal );
					return 1;
				}
				break;
			case '?': // Help
				help();
				return 0;
			default: // Undefined switch
				puts( NAME " - " VERSION );
				printf( "Unrecognized switch: -%s\n", s );
				printf( "sp0256 -? for help.\n" );
				return 1;
			}
		}
		else
		{
			sstr << " " << s;
			pistr = &sstr;
			mode = 'T';
		}

		if ( errno_ )
			break;
	}

	if ( !mode )
	{
		puts( NAME " - " VERSION );
		printf( "sp0256 -? for help.\n" );
	}

	int nAl2 = 0;
	int al2 = 0, lastal2 = 0, preval2 = 0; // signed int !!
	unsigned cnt = 0;
	int last = 0;
	int freq = xtal/2/156;
	int minSample = 0;
	int maxSample = 0;
	int bitsSample = 0;
	SystemClock systemClock;
	Win32Sleeper sleeper;

	if ( mode == 'T' )
	{
		// Load "Narrator" dictionary
		for ( size_t i=0; 
			i < ( model==_AL2 ? sp0256_al2::nlabels 
				: model==_012 ? sp0256_012::nlabels 
				: 0 ); 
			++i )
		{
			dict[ toUpperCase(
				  model==_AL2 ? sp0256_al2::labels[i] 
				: model==_012 ? sp0256_012::labels[i] 
				: "" ) ] = i;
		}
	}

	WaveWriter waveWriter;
	if ( !errno_ && waveFileName )
	{
		if ( waveFreq < freq )
			waveFreq = freq;
		errno_ = waveWriter.create( fileName = waveFileName, waveFreq, freq, 1, 8 );
	}

	if ( errno_ )
	{
		puts( NAME " - " VERSION );
		char buf[80];
		strerror_s( buf, errno_ );
		printf( "%s error: %s\n", fileName, buf );
		return 1;
	}

	int sample, *codes = 0;
	int codemax = 0;
	const char* *sp0256_labels = 0;

	if ( model == _AL2 )
	{
		codes = codes_al2;
		codemax = sp0256_al2::nlabels - 1;
		sp0256_labels = sp0256_al2::labels;
		sp0256_setLabels( sp0256_al2::nlabels, sp0256_al2::labels );
	} 
	else if ( model == _012 )
	{
		codes = codes_012;
		codemax = sp0256_012::nlabels - 1;
		sp0256_labels = sp0256_012::labels;
		sp0256_setLabels( sp0256_012::nlabels, sp0256_012::labels );
	}

	//FILE *out;

	//out = fopen( "spo256.out", "w" );

	if ( !waveFileName )
	{
		systemClock.setClockSpeed( freq );
		systemClock.setSleeper( &sleeper );
		systemClock.setAutoTurbo( false );
		outWaveInit();
		outWaveSetClockSpeed( freq );
		//outWaveSetClockSpeed( 3000 );
		outWaveReset();
	}

	if ( model == _AL2 )
		ivoice_init( sp0256_al2::mask );
	else if ( model == _012 )
		ivoice_init( sp0256_012::mask );

	while( !eos || !sp0256_halted() )
	{
		if ( !eos && sp0256_getStatus() ) 
		{
			std::string sym;
			switch ( mode )
			{
			case 'T':	// Text file mode
				for(;;)
				{
					int c = pistr->get();
					if ( pistr->eof() )
					{
						eos = 1;
						break;
					}
					
					if ( isalnum( c ) )
					{
						sym += char( toupper( c ) );
						const dict_t::const_iterator it = dict.find( sym );
						if ( it != dict.end() )
						{
							al2 = it->second;
							break;
						}
					}
					else
					{
						sym = "";
					}
				}
				break;
			case 'B':	// Binary file mode
				al2 = pistr->get() & 0x3F;
				if ( pistr->eof() )
				{
					eos = 1;
				}
				break;
			case 'A':	// Play all sounds/allophones
				al2 = nAl2++;
				if ( al2 > codemax )
					eos = 1;
				break;
			case 'D':	// Demo mode, play sample speech
			default:
				al2 = codes[nAl2++];
				if ( al2 < 0 )
					eos = 1;
				break;
			}

			if ( verbose ) {
				printf( "\t%8d %5d %2d", cnt, cnt-last, al2 );
				printf( "\t%2d = %5.1f ms - %s\n", preval2, (cnt-last)*1000./freq, sp0256_labels[preval2] );
			}

			if ( !eos )
			{
				if ( echo )
					printf( "%s ", sp0256_labels[al2] );

				last = cnt;
				preval2 = lastal2;
				lastal2 = al2;

				sp0256_sendCommand( uint32_t( al2 ) );
			}
		}

		sample = sp0256_getNextSample();
		bitsSample |= abs( sample );
		if ( sample < minSample )
			minSample = sample;
		if ( sample > maxSample )
			maxSample = sample;

		//fprintf( out, "%d\n", sample );
		sample >>= 8;
		sample += 0x80;
		sample &= 0xFF;

		//sample = abs( ( cnt & 0x7F ) - 0x40 ) + 0x60;
		//sample = abs( ( (cnt<<3) & 0xFF ) - 0x80 ) + 0x40;
		//sample = abs( ( (cnt<<2) & 0x1FF ) - 0x100 );

		if ( waveFileName )
		{
			waveWriter.write( sample );
		}
		else
		{
			outWave( uchar( sample ), uchar( sample ) );
			systemClock.runCycles( 1000 );
			outWaveCycles( 1 );
		}
		++cnt;
	}

	if ( waveFileName )
	{
		waveWriter.close();
	}
	else
	{
		outWaveFlush();
	}

	if ( verbose )
	{
		printf( "xtal=%d - freq=%d\n", xtal, freq );
		printf( "numSamples=%d - time=%8.4f s - minSample=%d - maxSample=%d - samplesMask=0x%X\n", cnt, cnt*1./freq, minSample, maxSample, bitsSample );
		puts( "Finished." );
	}

	_flushall(); // seems needed when redirecting stdout to file ...

	return 0;
}

