#pragma once

#include <exception>
#include <string>

class Exception : public std::exception
{
public:
	Exception(const std::string& message);
};