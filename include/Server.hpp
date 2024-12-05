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
	CLOSE,
	UNKNOWN
};

enum class e_complete_data
{
	CHUNKED_FINISHED,
	CHUNKED_UNFINISHED,
	COMPLETE,
	INCOMPLETE
};

typedef struct s_socket_data
{
    int fd;
    int port;
    e_socket_state state;
    Socket socket;
    std::stringstream buffer;

    s_socket_data(int fd, int port, e_socket_state state, Socket socket, std::stringstream&& buffer)
        : fd(fd), port(port), state(state), socket(socket), buffer(std::move(buffer)) {}
} t_socket_data;

class Server
{
	private:
		Config	&					_config;
		t_socket_data				_listening_socket;
		std::vector<t_socket_data>	_sockets;

		// Private methods
		void updatePoll();
		e_complete_data isDataComplete(t_socket_data &socket);

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