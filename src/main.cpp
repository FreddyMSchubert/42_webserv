#include "Server.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "Path.hpp"

#include <exception>
#include <execinfo.h>
#include <csignal>
#include <dlfcn.h>
#include <iostream>
#include <vector>
#include <signal.h>

#define LOCALHOST "127.0.0.1"

bool run = true;

void signalHandler(int signum)
{
	Logger::Log(LogLevel::INFO, "Interrupt signal (" + std::to_string(signum) + ") received. Exiting...");
	run = false;

	void *array[64];
	int size = backtrace(array, 64);
	Dl_info info;

	for (int i = 0; i < size; ++i) {
		if (dladdr(array[i], &info) && info.dli_sname) {
			fprintf(stderr, "%-3d %s + %td\n", i, info.dli_sname, (char *)array[i] - (char *)info.dli_saddr);
		} else {
			fprintf(stderr, "%-3d %p\n", i, array[i]);
		}
	}
}

std::vector<Config> parse_configs(std::string filename)
{
	// 1. Read the file
	std::string fileData;

	std::ifstream infile(filename, std::ios::in | std::ios::binary);
	if (!infile)
		throw std::runtime_error("Failed to open file: " + filename);
	std::ostringstream ss;
	ss << infile.rdbuf();
	infile.close();
	fileData = ss.str();

	// 2. Create Configs
	std::vector<Config> configs;
	std::regex serverBlockRegex(R"(server\s*\{([^}]*)\})");
	std::smatch match;
	std::string::const_iterator searchStart(fileData.cbegin());
	while (std::regex_search(searchStart, fileData.cend(), match, serverBlockRegex))
	{
		std::string serverConfig = match[1].str();
		configs.emplace_back(serverConfig);
		searchStart = match.suffix().first;
	}

	return configs;
}

int main(int argc, char *argv[])
{
	std::vector<Config> configs;

	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	else if (argc == 2)
	{
		configs = parse_configs(argv[1]);
		if (configs.size() == 0)
			return 1;
	}
	else
	{
		configs = parse_configs("./www/default.conf");
		if (configs.size() == 0)
			return 1;
	}

	srand(time(NULL));

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGABRT, signalHandler);

	Logger::Log(LogLevel::STAGE, "Signals successfully initialized!");

	if (configs.size() == 0)
	{
		Logger::Log(LogLevel::ERROR, "Failed to initialize configs");
		return 1;
	}

	if (configs.size() == 0) return 1;

	std::vector<Server> servers;

	try
	{
		for (auto &config : configs)
			servers.emplace_back(config);

		while (run)
			for (auto &server : servers)
				server.Run();
	}
	catch(const std::exception& e)
	{
		Logger::Log(LogLevel::ERROR, e.what());
		return 1;
	}

	return 0;
}
