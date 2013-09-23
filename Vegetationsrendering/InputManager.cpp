#include "InputManager.h"


InputManager::InputManager() : resetDevice(false)
{
}


void InputManager::init()
{
	Mouse::init();
	Keyboard::init();
}


void InputManager::update(const GameTime& gameTime)
{
	if (resetDevice)
	{
		resetDevice = false;

		if (!Mouse::isVisible())
			Mouse::setPosition(Window::getCenter());
	}

	Keyboard::update();
	Mouse::update();
}


void InputManager::onResetDevice()
{
	resetDevice = true;
}