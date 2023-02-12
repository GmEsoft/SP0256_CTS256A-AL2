#pragma once

// SYSTEM API

class System_I
{
public:
	virtual void run() = 0;
	virtual void stop() = 0;
	virtual void exit() = 0;
	virtual void reset() = 0;
	virtual void wakeup() = 0;
	virtual void step() = 0;
	virtual void callstep() = 0;

	// destructor
	virtual ~System_I()
	{
	}
};
