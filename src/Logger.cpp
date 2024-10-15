#include "Logger.hpp"

void Logger::Log(LogLevel level, const std::string &message)
{
	std::string current_time("[");

	time_t now = time(0);
	tm *ltm = localtime(&now);
	current_time += std::to_string(1900 + ltm->tm_year) + "-";
	current_time += std::to_string(1 + ltm->tm_mon) + "-";
	current_time += std::to_string(ltm->tm_mday) + " ";
	current_time += std::to_string(ltm->tm_hour) + ":";
	current_time += std::to_string(ltm->tm_min) + ":";
	current_time += std::to_string(ltm->tm_sec) + "]\t";

	std::string prefix("\033[1m");
	switch (level)
	{
		case LogLevel::INFO:
			prefix = "\033[32m" + current_time + "[ INFO ]" + "\033[22m";
			break;
		case LogLevel::WARNING:
			prefix = "\033[33m" + current_time + "[ WARN ]" + "\033[22m";
			break;
		case LogLevel::ERROR:
			prefix = "\033[31m" + current_time + "[ERROR!]" + "\033[22m";
			break;
	}

	std::string log_msg = prefix + " " + message + "\033[0m";

	if (level == LogLevel::ERROR)
		std::cerr << log_msg << std::endl;
	else
		std::cout << log_msg << std::endl;
}