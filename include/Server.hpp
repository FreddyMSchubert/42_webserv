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
		t_server_configs config;
		std::vector<Socket> _sockets;

	public:
		Server(t_server_block config);
		Server(Server const &src) = delete;
		Server &operator=(Server const &src) = delete;
		Server(Server&& other) noexcept = default;
		Server& operator=(Server&& other) noexcept = default;
		~Server() = default;
		
		void Run();
};