#pragma once

#include "Enums.hpp"
#include "Socket.hpp"

#include <iostream>
#include <vector>

class Server
{
	private:
		int _port;
		bool _run;
		std::string _config_file;
		std::vector<Socket> _sockets;

	public:
		Server(const std::string &config_file, int port);
		~Server();
		
		void Run();
};