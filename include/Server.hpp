#pragma once

#include "Enums.hpp"
#include "Socket.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "SocketManager.hpp"

#include <iostream>
#include <vector>
#include <fstream>

class Server
{
	private:
		// std::vector<Socket> _sockets;
		Config			&_config;
		SocketManager	_socketManager;

	public:
		Server(Config &config);
		Server(Server const &src) = delete;
		Server &operator=(Server const &src) = delete;
		Server(Server&& other) noexcept = default;
		~Server() = default;
		
		void Run();
};
