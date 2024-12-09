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

const std::string HTTP_MIN_HEADER_PATTERN = 
    "^(GET|POST|DELETE)\\s+(\\/[^\\s]*)?\\s+HTTP\\/1\\.1\\r\\n"
    "Host:\\s+[^\\r\\n]+\\r\\n"
    "([^\\r\\n]+\\r\\n)*"
    "\\r\\n";        

typedef struct s_socket_state
{
	bool read; // POLLIN
	bool write; // POLLOUT
	bool disconnect; // POLLHUP
	bool error; // POLLERR
}	t_socket_state;

enum class e_complete_data
{
	CHUNKED_FINISHED,
	CHUNKED_UNFINISHED,
	COMPLETE,
	INCOMPLETE
};

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

		// Private methods
		void updatePoll();
		e_complete_data isDataComplete(t_socket_data &socket);
		// void handleRequest(t_socket_data& socket);

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
