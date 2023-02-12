#ifndef MFCTERMAUDIO_H
#define MFCTERMAUDIO_H

#ifdef __cplusplus
# define CFUNC extern "C"
#else
# define CFUNC
#endif

CFUNC void queueWaveBuffer( unsigned char* Buffer, unsigned long BufSize );

CFUNC void initWaveOutDevice( unsigned long WaveFreq );

CFUNC void closeWaveOutDevice();

CFUNC long getWaveBuffersCounter();

CFUNC void setWaveBuffersCounter( long n );

#endif
