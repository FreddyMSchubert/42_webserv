#pragma once

#include <iostream>

enum class LogLevel
{
	INFO,
	WARNING,
	ERROR,
	STAGE
};

class Logger
{
	public:
		static void Log(LogLevel level, const std::string &message);
};
