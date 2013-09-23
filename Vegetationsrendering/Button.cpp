#include "Button.h"

char Button::get(const std::string& name)
{
	if (name == "VK_LEFT")
		return VK_LEFT;

	if (name == "VK_RIGHT")
		return VK_RIGHT;

	if (name == "VK_UP")
		return VK_UP;

	if (name == "VK_DOWN")
		return VK_DOWN;

	if (name == "VK_CONTROL")
		return VK_CONTROL;

	if (name == "VK_SHIFT")
		return VK_SHIFT;

	if (name == "VK_TAB")
		return VK_TAB;

	if (name == "VK_SPACE")
		return VK_SPACE;

	return Converter::convert<char>(name);
}