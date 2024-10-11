#include "../include/Server.hpp"
#include "../include/Logger.hpp"
#include "../include/Tests.hpp"
#include "../include/Config.hpp"

#include <iostream>
#include <vector>

std::vector<t_server_config> init_testing_configs()
{
	std::vector<t_server_config> configs(2);

	configs[0].server_names.push_back("clickergame.com");
	configs[1].server_names.push_back("platformergame.com");

	configs[0].host = "127.0.0.1";
	configs[1].host = "127.0.0.1";

	configs[0].port = 8080;
	configs[1].port = 8081;

	configs[0].default_location.allowed_methods.push_back(Method::GET);
	configs[1].default_location.allowed_methods.push_back(Method::GET);
	configs[1].default_location.allowed_methods.push_back(Method::POST);

	configs[0].default_location.root = "./www/clicker";
	configs[1].default_location.root = "./www/platformer";

	configs[0].default_location.index = "index.html";
	configs[1].default_location.index = "index.html";

	configs[0].default_location.directory_listing = false;
	configs[1].default_location.directory_listing = true;

	configs[0].default_location.client_max_body_size = 1000000;
	configs[1].default_location.client_max_body_size = 1000;

	configs[1].error_pages.push_back((t_error_page){404, (t_location){std::vector<Method>(), "./www/platformer/404", "404.html", false, 0}});

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

	// Init

	for (auto &config : configs)
	{
		try
		{
			servers.push_back(config);
			servers.back().Init();
		}
		catch(const std::exception& e)
		{
			Logger::Log(LogLevel::ERROR, e.what());
		}
	}

	// Run loop

	while (true)
	{
		try
		{
			for (auto &server : servers)
				server.Run();
		}
		catch(const std::exception& e)
		{
			Logger::Log(LogLevel::ERROR, e.what());
		}
	}

	return 0;
}
