#pragma once

// CLOCK API

class Clock_I
{
public:
	// do CPU cycles
	virtual void runCycles( long cycles ) = 0;

	// Set clock speed (kHz)
	virtual void setClockSpeed( long speed ) = 0;

	// Set RTC rate (kHz)
	virtual void setRtcRate( long rate ) = 0;

	// Set auto turbo
	virtual void setAutoTurbo( bool autoTurbo ) = 0;

	// destructor
	virtual ~Clock_I()
	{
	}
};
