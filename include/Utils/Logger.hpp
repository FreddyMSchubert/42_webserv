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
		static void Log(LogLevel level, int serverId, const std::string &message);
};
