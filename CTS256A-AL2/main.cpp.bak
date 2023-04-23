/*
    CTS256A-AL2 - Main Module.

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

#define NAME	"CTS256A-AL2(tm) Emulator"
#define VERSION	"v0.0.4-alpha"

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
		"cts256a-al2 [-iFile] [-t] [-b] [-e] [-d] [-v] [-n] [text]\n"
		" -iFile    Optional input filename\n"
		" -t        Select text output (allophone labels) (default)\n"
		" -b        Select binary output (range 40..7F)\n"
		" -e        Echo input text\n"
		" -v        Verbose mode\n"
		" -d        Debug mode\n"
		" -n        Suppress 'O.K.'\n"
		" --        Stop parsing options\n"
		" text      Optional text to convert\n"
		"If no -iFile and no text is given, reads input from stdin.\n"
		"Example: echo Hello World. | CTS256A-AL2.exe -n | SP0256.exe -i-\n"
	);
}

int _tmain(int argc, _TCHAR* argv[])
{
	char mode = 'T';
	bool echo = false, debug = false, verbose = false, noOK = false, opts = true;

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

		if ( opts && *s == '-' )
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
			case '-': // End opts
				opts = false;
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
			opts = false;
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

