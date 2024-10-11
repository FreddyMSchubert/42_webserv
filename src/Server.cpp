#include "../include/Server.hpp"

Server::Server(t_server_config config) : config(config) { }

void Server::Init()
{
	_sockets.reserve(1); // TODO: resize accoding to the config how many ips or ports we have to listen to

	Logger::Log(LogLevel::INFO, "Initializing new Webserv at \"" + config.host + ":" + std::to_string(config.port) + "\".");

	
	_sockets.emplace_back();

	for (auto &socket : _sockets)
		socket.Init(config);
}

void Server::Run()
{
	for (auto &socket : _sockets)
		socket.Run();
}
