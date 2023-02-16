# SP0256_CTS256A-AL2
G.I./Microchip SP0256 Speech Processor and CTS256A-AL2 Text-To-Speech Processor Emulation

Version 0.0.3-alpha.

Initial commit. More info to come.

## Description

Two emulators in this project:
- SP0256 Speech Processor, generating audio speech from allophones (-AL2: Narrator<sup>TM</sup>) or words (-012 Intellivoice<sup>TM</sup>);
- CTS256A-AL2 Text-To-Speech Processor, converting English text to allophones.

Two programs:
- SP0256.EXE (type -? for help);
- CTS256A-AL2.EXE (idem).


They can be combined together (the output from CTS256A-AL2 serves as input for SP0256):

`CTS256A-AL2 -I:README.MD | SP0256 -I-`


## Credits

J. Zbiciak, author of the Intellivoice emulator in C language, which this SP0256 emulator is based on.
