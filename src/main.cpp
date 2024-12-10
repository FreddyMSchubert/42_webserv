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
	std::vector<std::string> server_blocks;
	std::string token;
	int brace_count = 0;
	bool inside_server_block = false;

	for (size_t i = 0; i < fileData.size(); ++i)
	{
		char c = fileData[i];
		token += c;

		if (fileData.compare(i, 6, "server") == 0 && !inside_server_block)
		{
			inside_server_block = true;
			i += 5; // Skip "server"
		}
		else if (c == '{' && inside_server_block)
		{
			brace_count++;
		}
		else if (c == '}' && inside_server_block)
		{
			brace_count--;
			if (brace_count == 0)
			{
				server_blocks.push_back(token);
				token.clear();
				inside_server_block = false;
			}
		}
	}
	if (inside_server_block)
		throw std::runtime_error("Unmatched '{' in configuration file.");

	std::vector<Config> configs;
	for (const std::string &server_data : server_blocks)
	{
		size_t start = server_data.find('{');
		size_t end = server_data.rfind('}');
		if (start == std::string::npos || end == std::string::npos || end <= start)
			throw std::runtime_error("Invalid server block.");
		std::string server_content = server_data.substr(start + 1, end - start - 1);

		try
		{
			Config server_config(server_content);
			configs.push_back(server_config);
		}
		catch (const std::exception &e)
		{
			Logger::Log(LogLevel::ERROR, e.what());
		}
	}
	return (configs);
}

int main(int argc, char *argv[])
{
	std::vector<Config> configs;

	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	else
	{
		std::string configFilee = "./config/default.conf";
		if (argc == 2)
			configFilee = argv[1];
		try {
			configs = parse_configs(configFilee);
		} catch (std::exception & e){
			std::cout << e.what() << std::endl;
		}
		for (auto it1 = configs.begin(); it1 != configs.end(); ++it1)
		{
			for (auto it2 = it1 + 1; it2 != configs.end(); ++it2)
			{
				if (it1->getHost() == it2->getHost() && it1->getPort() == it2->getPort())
				{
					it2 = configs.erase(it2);
					it1 = configs.erase(it1);
					throw std::runtime_error("Host and Port are similar in minimum two servers!");
				}
			}
		}
	}

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
		{
			Logger::Log(LogLevel::INFO, "Initializing Server: " + config.getHost() + ":" + std::to_string(config.getPort()));
			servers.emplace_back(config);
		}

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
