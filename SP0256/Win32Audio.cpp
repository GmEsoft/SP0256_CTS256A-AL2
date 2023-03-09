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

#include "stdafx.h"
#include "windows.h"
#include "mmsystem.h"

#include "Win32Audio.h"

#include <deque>


static bool WaveDeviceInitialized = false;
static int WaveBuffersQueued = 0;
static long WaveBuffersCounter = 0;

static HWAVEOUT		hWaveOut;
static WAVEFORMATEX	waveform;

std::deque< PWAVEHDR > WaveHdrsToDelete;

static CRITICAL_SECTION* pCriticalSection = NULL;


// ============================================================================
// get MM Result Message
// ============================================================================
const char* getMMResultMsg( MMRESULT errCode )
{
	static char buf[80];
	switch( errCode )
	{
	case MMSYSERR_ALLOCATED:
		return "Specified resource is already allocated.";
	case MMSYSERR_BADDEVICEID:
		return "Specified device identifier is out of range. ";
	case MMSYSERR_NODRIVER:
		return "no device driver is present.";
	case MMSYSERR_NOMEM:
		return "Unable to allocate or lock memory. ";
	case MMSYSERR_NOERROR :
		return "waveOutopen is opened successful!";
	case MMSYSERR_INVALHANDLE:
		return "Specified device handle is invalid. ";
	case WAVERR_SYNC:
		return "The device is synchronous but waveOutOpen was called without using the WAVE_ALLOWSYNC flag. ";
	case WAVERR_BADFORMAT:
		return "Attempted to open with an unsupported waveform-audio format ";
	case WAVERR_STILLPLAYING:
		return "There are still buffers in the queue.";
	}
	sprintf_s( buf, sizeof(buf)-1, "Unknown error: 0x%04X (%d)", errCode, errCode );
	return buf;
}

MMRESULT MMErrorBox( MMRESULT errCode, char* context = NULL )
{
	if ( errCode )
	{
#ifdef _WINDOWS
		MessageBox( NULL, getMMResultMsg( errCode ), context, MB_OK );
#endif
#ifdef _CONSOLE
		printf( "Error %s:\n%s\nPress ENTER: ", context ? context : "", getMMResultMsg( errCode ) );
		getchar();
#endif
	}
	return errCode;
}

void CALLBACK waveOutProc(HWAVEOUT waveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	/* Has a buffer finished playing? */
	if ( uMsg == MM_WOM_DONE )
	{
#pragma warning(suppress:4312)
		PWAVEHDR pWaveHdr = (PWAVEHDR)dwParam1;

		EnterCriticalSection( pCriticalSection );
		WaveHdrsToDelete.push_back( pWaveHdr );
		LeaveCriticalSection( pCriticalSection );

		--WaveBuffersQueued;
		++WaveBuffersCounter;
	}
}

CFUNC void cleanWaveBufferQueue()
{
	PWAVEHDR pWaveHdr;

	EnterCriticalSection( pCriticalSection );

	while ( !WaveHdrsToDelete.empty() )
	{
		pWaveHdr = WaveHdrsToDelete.front();
		WaveHdrsToDelete.pop_front();
		MMErrorBox( waveOutUnprepareHeader( hWaveOut, pWaveHdr, sizeof( WAVEHDR ) ), "cleanWaveBufferQueue/waveOutUnprepareHeader" );
		delete pWaveHdr;
	}

	LeaveCriticalSection( pCriticalSection );
}

CFUNC void queueWaveBuffer( unsigned char* Buffer, unsigned long BufSize )
{
	PWAVEHDR pWaveHdr = new WAVEHDR;

	cleanWaveBufferQueue();

	if ( WaveDeviceInitialized )
	{

		if ( WaveBuffersQueued == 0 )
			MMErrorBox( waveOutPause( hWaveOut ), "queueWaveBuffer/waveOutPause" );
		else if ( WaveBuffersQueued == 1 )
			MMErrorBox( waveOutRestart( hWaveOut ), "queueWaveBuffer/waveOutRestart" );

		ZeroMemory( pWaveHdr, sizeof( WAVEHDR ) );

		pWaveHdr->lpData = (LPSTR)Buffer;
		pWaveHdr->dwBufferLength = BufSize;

		MMErrorBox( waveOutPrepareHeader( hWaveOut, pWaveHdr, sizeof( WAVEHDR ) ), "queueWaveBuffer/waveOutPrepareHeader" );

		MMErrorBox( waveOutWrite( hWaveOut, pWaveHdr, sizeof( WAVEHDR ) ), "queueWaveBuffer/waveOutWrite"  );

		++WaveBuffersQueued;
	}
}

CFUNC void closeWaveOutDevice()
{
	MMRESULT res;

	if ( WaveDeviceInitialized )
	{
		if ( WaveBuffersQueued == 1 )
			MMErrorBox( waveOutRestart( hWaveOut ), "queueWaveBuffer/waveOutRestart" );

		while ( WaveBuffersQueued )
			Sleep( 1 );

		MMErrorBox( waveOutReset( hWaveOut ), "closeWaveOutDevice/waveOutReset" );

		cleanWaveBufferQueue();

		while ( ( res = waveOutClose( hWaveOut ) ) == WAVERR_STILLPLAYING )
			Sleep( 1 );

		MMErrorBox( res, "closeWaveOutDevice/waveOutClose" );

		WaveDeviceInitialized = false;
	}
}

CFUNC void initWaveOutDevice( unsigned long WaveFreq )
{
	if ( !pCriticalSection )
	{
		pCriticalSection = new CRITICAL_SECTION;
		InitializeCriticalSection( pCriticalSection );
	}

	cleanWaveBufferQueue();

	closeWaveOutDevice();

	ZeroMemory( &waveform, sizeof( waveform ) );

	waveform.wFormatTag = WAVE_FORMAT_PCM;	/* format type */
	waveform.nChannels = 2;					/* number of channels (i.e. mono, stereo...) */
	waveform.nSamplesPerSec = WaveFreq;		/* sample rate */
	waveform.nAvgBytesPerSec = 2*WaveFreq;	/* for buffer estimation */
	waveform.nBlockAlign = 2;				/* block size of data */
	waveform.wBitsPerSample = 8;			/* number of bits per sample of mono data */

#pragma warning(suppress:4311)
	MMErrorBox( waveOutOpen( &hWaveOut, WAVE_MAPPER, &waveform, (DWORD)waveOutProc, 0, CALLBACK_FUNCTION ), "initWaveOutDevice/waveOutOpen" );

	WaveDeviceInitialized = true;
}

CFUNC long getWaveBuffersCounter()
{
	return WaveBuffersCounter;
}

CFUNC void setWaveBuffersCounter( long n )
{
	WaveBuffersCounter = n;
}
