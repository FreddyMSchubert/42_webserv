#include "Server.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include <iostream>
#include <vector>

std::vector<t_server_config> init_testing_configs()
{
	std::vector<t_server_config> configs(3);

	configs[0].server_names.push_back("clickergame.com");
	configs[1].server_names.push_back("platformergame.com");
	configs[2].server_names.push_back("teeetris.com");

	configs[0].host = "127.0.0.1";
	configs[1].host = "127.0.0.1";
	configs[2].host = "127.0.0.1";

	configs[0].port = 8080;
	configs[1].port = 8081;
	configs[2].port = 4242;

	// configs[0].port = 8082;
	// configs[1].port = 8083;
	// configs[2].port = 4243;

	configs[0].default_location.allowed_methods[Method::GET] = true;
	configs[1].default_location.allowed_methods[Method::GET] = true;
	configs[1].default_location.allowed_methods[Method::POST] = true;
	configs[2].default_location.allowed_methods[Method::GET] = true;

	configs[0].default_location.root = "./www/clicker";
	configs[1].default_location.root = "./www/platformer";
	configs[2].default_location.root = "./www/tetris";

	configs[0].default_location.index = "/index.html";
	configs[1].default_location.index = "/index.html";
	configs[2].default_location.index = "/index.html";

	configs[0].default_location.directory_listing = false;
	configs[1].default_location.directory_listing = true;
	configs[2].default_location.directory_listing = false;

	configs[0].default_location.client_max_body_size = 1000000;
	configs[1].default_location.client_max_body_size = 1000;
	configs[2].default_location.client_max_body_size = 1000000;

	configs[1].error_pages.push_back((t_error_page){404, (t_location){std::unordered_map<Method, bool>(), "./www/platformer/404", "404.html", false, 0}});

	configs[0].locations = std::vector<t_location>(2);
	configs[0].locations[0] = (t_location){std::unordered_map<Method, bool>(), "./www/clicker/assets", "", true, 0};
	configs[0].locations[0].allowed_methods[Method::GET] = false;
	configs[0].locations[1] = (t_location){std::unordered_map<Method, bool>(), "./www/clicker/assets/particles", "", true, 0};
	configs[0].locations[1].allowed_methods[Method::GET] = true;

	return configs;
}

bool run = true;

void signalHandler(int signum)
{
	Logger::Log(LogLevel::INFO, "Interrupt signal (" + std::to_string(signum) + ") received. Exiting...");
	run = false;
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		std::vector<t_sserver_config> configs = get_config(true, argv);
	else if (argc == 2)
		std::vector<t_sserver_config> configs = get_config(true, argv);
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
