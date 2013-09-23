#include "Exception.h"


Exception::Exception(const std::string& message) : std::exception(message.c_str())
{
}