#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include "Packets/Response.hpp"
#include "Packets/Request.hpp"
#include "Settings.hpp"
#include "../include/Config.hpp"

class Socket
{
	private:
		int _socket_pid;
		struct sockaddr_in _socket;
		std::vector<int> _clients;
		t_server_config config;
		void _connect();
		void _close();
		void _setNonBlocking(int fd);
	public:
		Socket(t_server_config config);
		Socket(int port, std::string address);
		Socket(Socket&& other) noexcept = default;
		Socket& operator=(Socket&& other) noexcept = default;
		Socket(const Socket &src) = delete;
		Socket &operator=(const Socket &src) = delete;
		~Socket();
		void sendData(Response &response, int socket_fd);
		void sendData(const std::string &data, int socket_fd);
		void Run();
		void closeSocket(int socket);
		void redirectToError(int client_fd, int error_code);
		void sendRedirect(int client_fd, const std::string& new_url);
};