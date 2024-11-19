#include "Server.hpp"

// TODO: function parameter configs are always const.. yes / no?

Server::Server(Config &config) : _config(config)
{
	_sockets.reserve(1); // TODO: resize accoding to the config how many ips or ports we have to listen to

	Logger::Log(LogLevel::INFO, "Initializing new Webserv at \"" + _config.getHost() + ":" + std::to_string(_config.getPort()) + "\".");

	_sockets.emplace_back(_config);
}

void Server::Run()
{
	for (auto &socket : _sockets)
		socket.Run();
}
