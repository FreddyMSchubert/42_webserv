#include "Server.hpp"

// TODO: function parameter configs are always const.. yes / no?

Server::Server(t_server_config config, std::vector<struct pollfd> &pollfds) : config(config), _pollfds(pollfds)
{
	_sockets.reserve(1); // TODO: resize accoding to the config how many ips or ports we have to listen to

	Logger::Log(LogLevel::INFO, "Initializing new Webserv at \"" + config.host + ":" + std::to_string(config.port) + "\".");

	_sockets.emplace_back(config);
}

void Server::Run()
{
	for (auto &socket : _sockets)
		socket.Run();
}
