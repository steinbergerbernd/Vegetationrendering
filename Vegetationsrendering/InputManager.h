#pragma once

#include "Input.h"
#include "Types.h"

class InputManager
{
public:
	InputManager();

	virtual void init();
	virtual void update(const GameTime& gameTime);

	virtual void onResetDevice();

private:
	unsigned buttonShowCursor;

	bool resetDevice;
};