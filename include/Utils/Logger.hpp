#pragma once

#include <iostream>

enum class LogLevel
{
	INFO,
	WARNING,
	ERROR
};

class Logger
{
	public:
		static void Log(LogLevel level, const std::string &message);
};
