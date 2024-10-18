#include "Server.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include <iostream>
#include <vector>

bool run = true;

void signalHandler(int signum)
{
	Logger::Log(LogLevel::INFO, "Interrupt signal (" + std::to_string(signum) + ") received. Exiting...");
	run = false;
}

int main(int argc, char *argv[])
{
	if (argc == 1 || argc == 2)
		t_sserver_configs configs = get_config(argv);
	else
		std::cerr << "Either use ./webserv to use the default.conf or ./webserv <your_conf>" << std::endl;

	srand(time(NULL));

	// signal(SIGINT, signalHandler);
	// signal(SIGTERM, signalHandler);

	// std::vector<Server> servers;

	// try
	// {
	// 	for (auto &config : configs)
	// 		servers.emplace_back(config);

	// 	while (run)
	// 		for (auto &server : servers)
	// 			server.Run();
	// }
	// catch(const std::exception& e)
	// {
	// 	Logger::Log(LogLevel::ERROR, e.what());
	// 	return 1;
	// }

	return 0;
}
