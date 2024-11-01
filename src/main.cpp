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

int main(int argc, char *argv[])
{
	std::vector<t_server_config>	configs;

	if (argc == 1 || argc == 2)
		configs = get_config(argv);
	else
		std::cerr << "Either use ./webserv to use the default.conf or ./webserv <your_conf>" << std::endl;

	srand(time(NULL));

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGABRT, signalHandler);

	std::cout << "Signals successfully initialized!" << std::endl;

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
