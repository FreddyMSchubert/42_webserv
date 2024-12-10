#pragma once

#include "Enums.hpp"
#include "Socket.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <array>

typedef struct s_socket_state
{
	bool read; // POLLIN
	bool write; // POLLOUT
	bool disconnect; // POLLHUP
	bool error; // POLLERR
}	t_socket_state;

typedef struct s_socket_data
{
	Socket socket;
	int fd;
	t_socket_state states;
	std::stringstream buffer;

	s_socket_data(int fd, Socket socket)
		: socket(socket), fd(fd), buffer(std::stringstream()) {};
	s_socket_data(Config &config)
		: socket(config), fd(socket.getSocketFd()), buffer(std::stringstream()) {};
} t_socket_data;

class Server
{
	private:
		Config			&			_config;
		t_socket_data				_listening_socket;
		std::vector<t_socket_data>	_sockets;

		void updatePoll();
		bool isDataComplete(t_socket_data &socket);
		// void handleRequest(t_socket_data& socket);

	public:
		Server(Config &config);
		Server(Server const &src) = delete;
		Server &operator=(Server const &src) = delete;
		Server(Server&& other) noexcept = default;
		~Server() = default;
		
		void Run(); // Main server loop
};
