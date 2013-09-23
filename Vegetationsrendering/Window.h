#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Point.h"

class Window
{
public:
	static void setHandle(HWND handle);
	static Point getCenter();

private:
	static HWND handle;
};