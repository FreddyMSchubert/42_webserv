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
#include <poll.h>
#include <cstring>
#include <algorithm>
#include <exception>
#include <string>

#include "Packets/Response.hpp"
#include "Packets/Request.hpp"
#include "Settings.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

class Socket
{
	private:
		int _socket_fd;
		struct sockaddr_in _socket;
		std::vector<struct pollfd> _clients;
		Config &_config;

	public:
		// Constructor and Destructor
		Socket(Config &config);
		Socket &operator=(const Socket& copy);
		~Socket();

		// Socket Setup
		int	 addNewSocket();
		void connectSocket();
		void setNonBlockingSocket(int fd);

		// Data Transmission
		void sendData(Response &response, int socket_fd);
		void sendData(const std::string &data, int socket_fd);
		std::string receiveData(int client_fd);

		// Connection Management
		void closeSocket(int socket);
		void closeAllSockets();
		void sendRedirect(int client_fd, const std::string& new_url);
		void redirectToError(int client_fd, int error_code);

		// Getters
		int getSocketFd() const { return _socket_fd; }
		int getPort() const { return _config.getPort(); }

		//Setters
		void setSocketFd(int socket_fd) { _socket_fd = socket_fd; }
};
