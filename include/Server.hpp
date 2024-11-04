#pragma once

#include "Enums.hpp"
#include "Socket.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#include <iostream>
#include <vector>
#include <fstream>

class Server
{
	private:
		t_server_config config;
		std::vector<Socket> _sockets;
		std::vector<struct pollfd> &_pollfds;

	public:
		Server(t_server_config config, std::vector<struct pollfd> &pollfds);
		Server(Server const &src) = delete;
		Server &operator=(Server const &src) = delete;
		Server(Server&& other) noexcept = default;
		Server& operator=(Server&& other) noexcept = default;
		~Server() = default;
		
		void Run();
};