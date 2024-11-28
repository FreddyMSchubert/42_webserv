#pragma once

#include "Enums.hpp"
#include "Socket.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

enum class e_socket_state
{
	READ,
	WRITE,
	ACCEPT,
	CONNECT,
	CLOSE
};

typedef struct s_socket_data
{
	int					fd;
	int					port;
	e_socket_state		state;
	Socket				&socket;
	std::stringstream	buffer;
}	t_socket_data;

class Server
{
	private:
		Config	&_config;
		std::vector<Socket>			_sockets;
		std::vector<t_socket_data>	_socket_data;

		// Private methods
		void updatePoll();
		bool isDataComplete(t_socket_data &socket);

	public:
		// Constructors and destructors
		Server(Config &config);
		Server(Server const &src) = delete;
		Server &operator=(Server const &src) = delete;
		Server(Server&& other) noexcept = default;
		~Server() = default;
		
		// Public methods
		void Run(); // Main server loop
};
