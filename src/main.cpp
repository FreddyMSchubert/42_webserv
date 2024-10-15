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

	configs[1].error_pages.push_back((t_error_page){404, (t_location){std::unordered_map<Method, bool>(), "./www/platformer/404", "404.html", false, 0, false}});

	configs[0].locations.push_back((t_location){std::unordered_map<Method, bool>(), "./www/clicker/assets", "", true, 0, false});
	configs[0].locations.push_back((t_location){std::unordered_map<Method, bool>(), "./www/clicker/assets/particles", "", true, 0, false});
	configs[0].locations.back().allowed_methods[Method::DELETE] = true;
	configs[0].locations.front().allowed_methods[Method::GET] = false;

	return configs;
}

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}

	srand(time(NULL));

	std::vector<t_server_config> configs = init_testing_configs(); // TODO: config parsing
	std::vector<Server> servers;

	try
	{
		for (auto &config : configs)
			servers.emplace_back(config);

		while (true)
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
