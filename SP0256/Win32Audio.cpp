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
