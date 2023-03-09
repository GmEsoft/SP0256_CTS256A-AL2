/*
    SP0256A - Win32 Audio Wave Output Interface.

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

#ifndef MFCTERMAUDIO_H
#define MFCTERMAUDIO_H

#ifdef __cplusplus
# define CFUNC extern "C"
#else
# define CFUNC
#endif

// Queue a wave buffer for playing
CFUNC void queueWaveBuffer( unsigned char* Buffer, unsigned long BufSize );

// Init the audio wave output device
CFUNC void initWaveOutDevice( unsigned long WaveFreq );

// Play the remaining buffer then close the audio wave output device
CFUNC void closeWaveOutDevice();

// Get the number of currently queued wave buffers.
CFUNC long getWaveBuffersCounter();

// Set the number of currently queued wave buffers.
CFUNC void setWaveBuffersCounter( long n );

#endif
