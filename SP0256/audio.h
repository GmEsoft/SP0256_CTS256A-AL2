#pragma once

#include "types.h"

// Update wave buffers and send them to audio device
void outWaveUpdate();

// Post next wave sample (left, right)
void outWave( uchar levelL, uchar levelR );

// Reset wave levels and cycles counter
void outWaveReset();

// Init audio device
void outWaveInit();

// Add cycles to the cycles counter
void outWaveCycles( ulong cycles );

// Set the logical waves sampling frequency
void outWaveSetClockSpeed( long speed );

// Flush the buffers to the audio device then close it
void outWaveFlush();
