#include "Mouse.h"


InputDevice<MouseState> Mouse::mouse;
float  Mouse::smoothing;
bool Mouse::visible = true;
unsigned Mouse::bufferIndex = 0;
Mouse::Buffer Mouse::buffer = Mouse::Buffer(10);


void Mouse::setState(const MouseState& mouseState)
{
	MouseState& next = mouse.getState(mouse.nextState);

	int delta = next.mouseWheelDelta;
	next = mouseState;
	next.mouseWheelDelta += delta;
}


void Mouse::init()
{
	smoothing = Config::getValue<float>(ConfigKeys::mouseSmoothing);

	setPosition(Window::getCenter());
	setVisible(false);
}


void Mouse::update()
{
	mouse.update();

	mouse.getState(mouse.nextState).mouseWheelDelta = 0;

	if (isVisible())
		return;

	Point center = Window::getCenter();
	Point position = getPosition();

	bufferIndex = (bufferIndex + 1) % buffer.size();
	buffer[bufferIndex] = Vector2((float)(position.x - center.x), (float)(position.y - center.y));

	setPosition(center);
}


bool Mouse::isLeftButtonDown()
{
	return mouse.getState(mouse.currState).leftButtonDown;
}


bool Mouse::isVisible()
{
	return visible;
}


bool Mouse::isRightButtonDown()
{
	return mouse.getState(mouse.currState).rightButtonDown;
}


const Point& Mouse::getPosition()
{
	return mouse.getState(mouse.currState).position;
}


bool Mouse::isRightButtonClick()
{
	return mouse.getState(mouse.currState).rightButtonDown && !mouse.getState(mouse.prevState).rightButtonDown;
}


bool Mouse::isLeftButtonClick()
{
	return mouse.getState(mouse.currState).leftButtonDown && !mouse.getState(mouse.prevState).leftButtonDown;
}


bool Mouse::isLeftButtonClickEnd()
{
	return mouse.getState(mouse.prevState).leftButtonDown && !mouse.getState(mouse.currState).leftButtonDown;
}


bool Mouse::isRightButtonClickEnd()
{
	return mouse.getState(mouse.prevState).rightButtonDown && !mouse.getState(mouse.currState).rightButtonDown;
}


int Mouse::getMouseWheelDelta()
{
	return mouse.getState(mouse.currState).mouseWheelDelta;
}


void Mouse::setPosition(const Point& position)
{
	SetCursorPos(position.x, position.y);

	mouse.getState(mouse.currState).position = position;
	mouse.getState(mouse.nextState).position = position;
}


void Mouse::setVisible(bool visible)
{
	Mouse::visible = visible;
	ShowCursor(visible);

	if (visible)
		buffer.assign(buffer.size(), Vector2::zero);
}


Vector2 Mouse::getMovement()
{
	Vector2 sumMovements;
	float sumWeights = 0.0f;
	float weight = 1.0f;

	for (unsigned i = 0; i < buffer.size(); i++)
	{
		sumWeights += weight;
		sumMovements += buffer[(bufferIndex - i + buffer.size()) % buffer.size()] * weight;
		weight *= smoothing;
	}

	return sumMovements / sumWeights;
}
