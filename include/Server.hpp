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
#include <chrono>

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
	std::chrono::time_point<std::chrono::steady_clock> last_activity;

	s_socket_data(int fd, Socket socket) // Client socket constructor
		: socket(std::move(socket)), fd(fd), buffer(), last_activity(std::chrono::steady_clock::now()) {}
	s_socket_data(Config &config) // Listening socket constructor
		: socket(config), fd(socket.getSocketFd()), buffer() {}
	s_socket_data(const s_socket_data&) = delete;
	s_socket_data& operator=(const s_socket_data&) = delete;

	s_socket_data(s_socket_data&& other) noexcept
		: socket(std::move(other.socket)), fd(other.fd), states(other.states), buffer(std::move(other.buffer)), last_activity(other.last_activity) {}
	s_socket_data& operator=(s_socket_data&& other) noexcept
	{
		if (this == &other)
			return *this;
		socket = std::move(other.socket);
		fd = other.fd;
		states = other.states;
		buffer = std::move(other.buffer);
		return *this;
	}
}	t_socket_data;

class Server
{
	private:
		Config			&			_config;
		t_socket_data				_listening_socket;
		std::vector<t_socket_data>	_sockets;

		void updatePoll();
		bool isDataComplete(t_socket_data &socket);
		void acceptNewConnections();
		void handleExistingConnections();

	public:
		Server(Config &config);
		Server(Server const &src) = delete;
		Server &operator=(Server const &src) = delete;
		Server(Server&& other) noexcept = default;
		~Server() = default;
		
		void Run(); // Main server loop
};
