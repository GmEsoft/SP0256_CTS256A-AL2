# SP0256_CTS256A-AL2

**G.I./Microchip SP0256 Speech Processor and CTS256A-AL2 Text-To-Speech Processor Emulation**

Based on the IntelliVoice emulator by Joe Zbiciak.

Version 0.0.6-alpha.

A pre-release of the executables (Win32-x86) can be found under 'Releases'.


## History

### CTS256A-AL2 v0.1.0-alpha
- added rules debugging mode `-r`;
- use 'external' RAM for buffers;
- fixed an issue with the idle loops handling;
- convert the input chars to upper case to circumvent a defect in the ROM related to the lower case chars handling.

### v0.0.6-alpha
- Migrated to Visual Studio 2017 => new solution file `SP0256_VS2005.sln` for compatibility with VS2005
  - Use `SP0256_VS2005.sln` to build under VS2005;
  - Use `SP0256.sln` to build under VS2017 (or higher -- to check).

### v0.0.5-alpha
- CTS256A_AL2: force output of each allophone as soon as possible;
- SP0256: input labels are case-insensitive.

### v0.0.4-alpha
- added GPL3 headers;
- solved compiler warnings;
- updated README.md .

### v0.0.3-alpha
- SP0256: `-w[wavFreq]:file.wav`;
- SP0256: `-DT` = trace; `-DD` = single-step debug, `-DS` = samples;
- CTS256A-AL2: updated debugger help text;
- CTS256A-AL2: code clean-up.


### v0.0.1-alpha
- Initial commit


## Description

Two emulators in this project:
- SP0256 Speech Processor, generating audio speech from allophones (-AL2: Narrator<sup>TM</sup>) or words (-012 Intellivoice<sup>TM</sup>);
- CTS256A-AL2 Text-To-Speech Processor, converting English text to allophones.

Two programs:
- SP0256.EXE (type -? for help);
- CTS256A-AL2.EXE (idem).


They can be combined together (the output from CTS256A-AL2 serves as input for SP0256):

`CTS256A-AL2 -I:README.MD | SP0256 -I-`

This project can currently be built using Microsoft Visual Studio 2005 using the solution file `SP0256_VS2005.sln`,
or Microsoft Visual Studio 2017 and onwards using the solution file `SP0256.sln`.


## SP0256.EXE

**SP0256(tm) Emulator**

**GI/Microchip SP0256-AL2 Narrator(tm) and SP0256-012 Intellivoice(tm) Speech Processor**


This program accurately emulates the SP0256 micro-sequencer and the voice generator, using the embedded 
original speech ROM images. The voice generator consists of a glottal pulse and noise generator, and
a 12-pole digital filter.


Two versions of the speech ROM are included:
- AL2 = the Narrator(tm) version, with 5 pauses and 59 allophones;
- 012 = the version included in the IntelliVoice cartridge for the IntelliVision game console.

Specify `-mAL2` (default) to select the AL2 Narrator(tm) speech ROM, or `-m012` to select the IntelliVoice
speech ROM.


The input can either be stdin or a file. Specify `-i-` to use stdin, or `-iFilename` to use an input file.
The format can either be ASCII (the allophone or pause labels for AL2, or the speech words for 012), 
or binary (6-bit addresses - the higher bits are masked out). Specify `-t` for ASCII text mode (the default), 
or `-b` for binary mode.


The output can either be the default sound output device, or a .WAV file. Specify `-wWavFile` to write the audio
stream to WavFile. Specify `-wFreq:WavFile` to generate a WAV file at a sampling frequancy other than the default.
The audio wave file format will be 8-bit PCM mono.

The XTAL frequency can also be specified via the option `-xXtal`, where 1000000 <= Xtal <= 5000000. The default
value for Xtal is 3120000 (3.12 MHz).

The audio sampling frequency is given by the expression `Freq = Xtal / 312`. So, for the default Xtal frequency of 3.12 MHz,
the audio sampling frequency will be 10 kHz. This audio sampling frequency will be the default frequency for the .WAV 
file if no other value is specified via the `-w` option.

Some echo/verbose/debugging flags are provided:
- `-e` to echo the generated allophones, pauses or words;
- `-v` to display more info about the generated allophones, pauses or words;
- `-dD` to trace the micro-sequencer instructions and allow single-stepping;
- `-dT` to trace the micro-sequencer instructions;
- `-dS` to display the generated waveforms.


Usage:
````
sp0256 [-m{AL2|012}] [-e] [-v] [-xClockFreq] [ -t | -b | -a ] [ -i{inFile|-} ] [-wWavFile]
-mAL2     Select Narrator(tm) speech ROM
-m012     Select Intellivoice speech ROM
-e        Echo speech elements (words or allophones)
-v        Verbose mode
-d[D|S|T] Set debug for [D]ebug, [S]amples or [T]race
-xClkFreq Xtal Clock Frequency in Hz (range: 1000000..5000000)
-iInFile  Say File
-i-       Say from stdin: echo ... | sp0256 -i-
-t        Text Mode (labels) (default)
-b        Binary Mode (addresses)
-a        Pronounce all words or allophones in speech ROM
-wWavFile Create .wav file
````


## CTS256A-AL2.EXE

**CTS256A-AL2(tm) Emulator**

**GI/Microchip CTS256A-AL2(tm) Code-To-Speech Speech Processor**


This program accurately emulates the CTS256A-AL2 companion chip of the SP0256-AL2 speech processor, 
using the embedded original code-to-speech ROM image.


The input can either be stdin, or a file, or the command line. Specify `-i-` to use stdin, `-iFilename` to use 
an input file, or put the text at the end of the command line, after the options.


The output is stdout. It can be piped to SP0256.exe in order to directly pronounce the allophones converted from
the input text. The format can either be ASCII (the allophone or pause labels), or binary (6-bit addresses OR-ed 
with 0x40). Specify `-t` for ASCII text mode (the default), or `-b` for binary mode.


Some echo/verbose/debugging flags are provided:
- `-e` to echo the input text;
- `-v` to echo the converted allophone labels;
- `-d` to enter debug mode.


The CTS256A-AL2 normally generates the output for 'O-K' on startup. Specify `-n` to suppress that.


Usage:
````
cts256a-al2 [-iFile] [-t] [-b] [-e] [-d] [-v] [-n] [text]
 -iFile    Optional input filename
 -t        Select text output (allophone labels) (default)
 -b        Select binary output (range 40..7F)
 -e        Echo input text
 -v        Verbose mode
 -d        Debug mode
 -n        Suppress 'O.K.'
 --        Stop parsing options
 text      Optional text to convert to speech
````

If no `-ifile` and no `text` is specified on the command line, reads input from stdin.
<br>
Example: `echo Hello World. | CTS256A-AL2.exe -n | SP0256.exe -i-`


## Useful links

### CTS256A-AL2

Data Sheet: http://bitsavers.informatik.uni-stuttgart.de/components/gi/speech/General_Instrument_-_AN-0505D_-_CTS256A-AL2_Code-to-Speech_Chipset_-_10Dec1986.pdf

Binary ROM image: https://github.com/palazzol/TMS7xxx_dumper/blob/main/software/dumps/cts256a.bin
from the GitHub project: https://github.com/palazzol/TMS7xxx_dumper

### SP0256-AL2 ROM Image

http://spatula-city.org/~im14u2c/sp0256-al2/

This image has been byte-reversed. It should be de-reversed in order to
incorporate this into ivoice.c in `ivoice.c` in project https://github.com/libretro/FreeIntv .


### Intellivoice Hardware Overview

http://spatula-city.org/~im14u2c/intv/tech/ivoice.html

Note that the mask ROM is Intellivoice specific, not the SP0256-AL2 !


### FreeIntv

https://github.com/libretro/FreeIntv

Note that the mask ROM is Intellivoice specific, not the SP0256-AL2 !


### CPCwiki

Front page: https://www.cpcwiki.eu/index.php/SP0256

Voice generator: https://www.cpcwiki.eu/index.php/SP0256_Voice_Generator

Allophones: https://www.cpcwiki.eu/index.php/SP0256_Allophones

Instruction set: https://www.cpcwiki.eu/index.php/SP0256_Instruction_Set

Measured timings: https://www.cpcwiki.eu/index.php/SP0256_Measured_Timings



## Next steps and ideas

### SP0256.EXE

- Implement SP0256-017, the talking clock, as soon as I get the internal and external ROM images.

### CTS256A-AL2.EXE

- Implement support for external ROMs.

### Other

- Implement a TCP socket server;
- Port to ESP32 FabGL.



## Copyright notice

Microchip, Inc. holds the copyrights to the SP0256-AL2 design and ROM Image, and to the CTS256A-AL2 ROM Image.
Microchip retains the intellectual property rights to the algorithms and data the emulated devices SP0256 and CTS256A-AL2 contain.



## Credits

Joe Zbiciak, author of the Intellivoice emulator in C language, which this SP0256 emulator is based on.

Frank Palazzolo, who designed an extractor to dump the masked ROMs of TMS7000-based devices, and published
the ROM binary image `cts256a.bin`.
