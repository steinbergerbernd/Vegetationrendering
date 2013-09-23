#pragma once

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Exception.h"
#include "Exceptions.h"
#include "Converter.h"

class Button
{
public:
	static char get(const std::string& name);
};