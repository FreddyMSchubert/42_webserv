#include "../include/Server.hpp"
#include <fstream>

Server::Server(const std::string &config_file, int port) : _port(port), _run(false), _config_file(config_file) { }

Server::~Server() {}

void Server::Run()
{

	_sockets.reserve(1); // TODO: resize accoding to the config how many ips or ports we have to listen to

	if (!std::fstream(_config_file).good())
	{
		throw std::runtime_error("Config file not found");
	}

	std::cout << "Running Webserv on port " << _port << " with config file " << _config_file << std::endl;

	_run = true;

	_sockets.emplace_back(_port, "127.0.0.1");

	for (auto &socket : _sockets)
		socket.Run();

	// Socket socket(_port, "127.0.0.1");
	// socket.Run();
	
	while (_run)
	{
		
	}

	std::cout << "Webserv stopped" << std::endl;

}