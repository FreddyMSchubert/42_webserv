#include "../include/Server.hpp"
#include <fstream>

Server::Server(const std::string &config_file, int port) : _port(port), _run(false),  _config_file(config_file) { }

Server::~Server() {}

void Server::Run()
{

	if (!std::fstream(_config_file).good())
	{
		throw std::runtime_error("Config file not found");
	}

	std::cout << "Running Webserv on port " << _port << " with config file " << _config_file << std::endl;

	_run = true;
	int i = 0;
	while (_run)
	{
		if (i++ == __INT_MAX__)
		{
			_run = false;
			break;
		}
		// Do stuff
	}

	std::cout << "Webserv stopped" << std::endl;

}