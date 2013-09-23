#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <map>
#include "ConfigKeys.h"
#include "Exception.h"
#include "Exceptions.h"
#include "Converter.h"

class Config
{
	typedef std::map<std::string, std::string> ConfigCollection;

public:
	template <class T>
	static T getValue(const std::string& key);

	static const std::string& getValue(const std::string& key);

	static void addFile(const std::string& path);

private:
	static ConfigCollection config;
};


template <class T>
T Config::getValue(const std::string& key)
{
	ConfigCollection::const_iterator it;

	if ((it = config.find(key)) == config.end())
		throw Exception(Exceptions::configKeyNotFound);

	return Converter::convert<T>(it->second);
}