#pragma once

#include "types.h"

void outWaveUpdate();
void outWave( uchar levelL, uchar levelR );
void outWaveReset();
void outWaveInit();
void outWaveCycles( ulong cycles );
void outWaveSetClockSpeed( long speed );
void outWaveFlush();
