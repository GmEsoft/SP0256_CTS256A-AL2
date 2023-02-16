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
