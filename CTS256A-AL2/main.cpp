// CTS256A-AL2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define NAME	"CTS256A-AL2(tm) Emulator"
#define VERSION	"v0.0.3-alpha"

#include "ConIOConsole.h"
#include "CTS256A_AL2.h"
#include "ConsoleDebugger.h"
#include "TMS7000CPU.h"
#include "TMS7000DebugHelper.h"
#include "TMS7000Disassembler.h"

#include <sstream>
#include <fstream>
#include <memory>

void help()
{
	puts(
		"GI/Microchip CTS256A-AL2(tm) Code-To-Speech Speech Processor\n\n"
		"Usage:\n"
		"cts256a-al2 [-Ifile] [-T] [-B] [-E] [-D] [-V] [-N] [text]\n"
		" -Ifile:   Optional input filename\n"
		" -T:       Select text output (allophone labels) (default)\n"
		" -B:       Select binary output (range 40..7F)\n"
		" -E:       Echo input text\n"
		" -V:       Verbose mode\n"
		" -D:       Debug mode\n"
		" -N:       Suppress 'O.K.'\n"
		" text      Optional text to convert\n"
		"If no -Ifile or text is specified, reads input from stdin\n"
		"Example: echo Hello World. | CTS256A-AL2.exe -N | SP0256.exe -T-\n"
	);
}

int _tmain(int argc, _TCHAR* argv[])
{
	char mode = 'T';
	bool echo = false, debug = false, verbose = false, noOK = false;

	std::istream *pistr = &std::cin;
	std::ostream *postr = &std::cout;

	std::stringstream sstr;
	std::auto_ptr< std::fstream > pfstr;
	
	ConIOConsole console;
	console.puts( NAME " - " VERSION "\n\n" );

	for ( int i=1; i<argc; ++i )
	{
		char *s = argv[i];
		char c = 0;

		if ( *s == '-' )
		{
			++s;
			switch ( toupper( *s ) )
			{
			case 'I':
				++s;
				if ( *s == ':' )
					++s;
				pfstr.reset( new std::fstream( s, std::fstream::in ) );
				if ( !pfstr->is_open() )
				{
					console.printf( "Failed to open %s\n", s );
					return 1;
				}
				pistr = pfstr.get();
				noOK = true;
				break;
			case 'B': // Bin file
				++s;
				mode = 'B';
				break;
			case 'T': // Text file
				++s;
				mode = 'T';
				break;
			case 'E': // Echo
				echo = 1;
				break;
			case 'D': // Debug
				debug = 1;
				break;
			case 'V': // Verbose
				verbose = 1;
				break;
			case 'N': // No OK
				noOK = 1;
				break;
			case '?': // Help
				help();
				return 0;
			default: // Undefined switch
				printf( "Unrecognized switch: -%s\n", s );
				printf( "sp0256 -? for help.\n" );
				return 1;
			}
		}
		else
		{
			sstr << " " << s;
			pistr = &sstr;
			noOK = true;
		}
	}

	CTS256A_AL2 system( *pistr, *postr );

	system.setOption( 'D', debug );
	system.setOption( 'E', echo );
	system.setOption( 'V', verbose );
	system.setOption( 'N', noOK );
	system.setOption( 'M', mode );

	system.run();
	
	console.puts( "Conversion complete.\n\n" );

	return 0;
}

