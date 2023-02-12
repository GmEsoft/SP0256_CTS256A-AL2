#pragma once

class IRQ_I
{
public:

	IRQ_I(void)
	{
	}

	virtual ~IRQ_I(void)
	{
	}

	virtual void trigger() = 0;
};
