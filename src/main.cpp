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

void	print_configs(std::vector<t_server_config> configs)
{
	for (size_t i = 0; i < configs.size(); i++)
	{
		std::cout << "host: " << configs[i].host << std::endl;
		std::cout << "port: "<< configs[i].port << std::endl;
		for (size_t j = 0; j < configs[i].server_names.size(); j++)
			std::cout << "server_names: " << configs[i].server_names[j] << std::endl;
		std::cout << "root_dir: " << configs[i].root_dir << std::endl;
		std::cout << "index_file: " << configs[i].index_file << std::endl;
		std::cout << "client_max_body_size: " << configs[i].client_max_body_size << std::endl;
		for (auto it = configs[i].error_pages.begin(); it != configs[i].error_pages.end(); ++it)
			std::cout << "Key_error_page: " << it->first << ", Value_error_page: " << it->second.asUrl() << std::endl;
		for (size_t m = 0; m < configs[i].locations.size(); m++)
		{
			std::cout << "location_path: " << configs[i].locations[m].path << std::endl;
			std::cout << "location_root_dir: " << configs[i].locations[m].root_dir << std::endl;
			for (auto it = configs[i].locations[m].allowed_methods.begin(); it != configs[i].locations[m].allowed_methods.end(); ++it)
				std::cout << "Key_location_allowed_methods: " << it->first << ", Value_location_allowed_methods: " << it->second << std::endl;
			std::cout << "location_directory_listening" << configs[i].locations[m].directory_listing << std::endl;
			std::cout << "location_upload_dir: " << configs[i].locations[m].upload_dir << std::endl;
		}
		std::cout << "===============================================================" << std::endl;
	}
}

int main(int argc, char *argv[])
{
	std::vector<t_server_config>	configs;

	srand(time(NULL));

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGABRT, signalHandler);

	if (argc == 1 || argc == 2)
		configs = get_config(argv, configs);
	else
		std::cerr << "Either use ./webserv to use the default.conf or ./webserv <your_conf>" << std::endl;


	std::cout << "Signals successfully initialized!" << std::endl;

	if (configs.size() == 0) return 1;

	// print_configs(configs);
	// sleep(2);
	
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
		signalHandler(69);
		return 1;
	}

	return 0;
}
