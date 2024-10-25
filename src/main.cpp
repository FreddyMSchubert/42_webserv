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

std::vector<t_server_config> init_testing_configs()
{

	try
	{
		std::vector<t_server_config> configs(3);
	
		configs[0].server_names.push_back("clickergame.com");
		configs[1].server_names.push_back("platformergame.com");
		configs[2].server_names.push_back("teeetris.com");

		configs[0].host = LOCALHOST;
		configs[1].host = LOCALHOST;
		configs[2].host = LOCALHOST;

		configs[0].port = 8080;
		configs[1].port = 8081;
		configs[2].port = 4242;

		configs[0].default_location.allowed_methods[Method::GET] = true;
		configs[1].default_location.allowed_methods[Method::GET] = true;
		configs[1].default_location.allowed_methods[Method::POST] = true;
		configs[2].default_location.allowed_methods[Method::GET] = true;

		configs[0].default_location.root = "./www/clicker/";
		configs[1].default_location.root = "./www/platformer/";
		configs[2].default_location.root = "./www/tetris/";

		configs[0].default_location.index = "/index.html";
		configs[1].default_location.index = "/index.html";
		configs[2].default_location.index = "/index.html";

		configs[0].default_location.directory_listing = false;
		configs[1].default_location.directory_listing = true;
		configs[2].default_location.directory_listing = false;

		configs[0].default_location.client_max_body_size = 1000000;
		configs[1].default_location.client_max_body_size = 1000;
		configs[2].default_location.client_max_body_size = 1000000;

		configs[1].error_pages.push_back((t_error_page){404, (t_location){std::unordered_map<Method, bool>(), "/www/platformer/404/", "404.html", false, 0}});

		configs[0].locations = std::vector<t_location>(2);

		configs[0].locations[0] = (t_location){std::unordered_map<Method, bool>(), "./www/clicker/assets/", "", true, 0};
		configs[0].locations[0].allowed_methods[Method::GET] = false;
		configs[0].locations[1] = (t_location){std::unordered_map<Method, bool>(), "./www/clicker/assets/particles/", "", true, 0};
		configs[0].locations[1].allowed_methods[Method::GET] = true;

		configs[0].port = rand() % 1000 + 8000;
		configs[1].port = rand() % 1000 + 8000;
		configs[2].port = rand() % 1000 + 8000;

		return configs;

	} catch (std::exception &e)
	{
		Logger::Log(LogLevel::ERROR, e.what());
		signalHandler(SIGABRT);
		return std::vector<t_server_config>(0);
	}
}

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}

	srand(time(NULL));

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGABRT, signalHandler);

	std::cout << "Signals successfully initialized!" << std::endl;

	std::vector<t_server_config> configs = init_testing_configs(); // TODO: config parsing

	std::cout << "configs.size() = " << configs.size() << std::endl;

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
