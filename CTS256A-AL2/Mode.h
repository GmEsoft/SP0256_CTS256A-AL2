#pragma once

// EXECUTION MODE MANAGER

enum ExecMode
{
	MODE_STOP = 0,
	MODE_STEP,
	MODE_CALL,
	MODE_RET,
	MODE_EXEC,
	MODE_RUN,
	MODE_RESET,
	MODE_EXIT = 99
};

class Mode
{
public:
	ExecMode getMode() const
	{
		return mode_;
	}

	void setMode( ExecMode mode )
	{
		mode_ = mode;
	}

private:
	ExecMode mode_;
};
