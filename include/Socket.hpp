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
		int _socket_fd = -1;
		struct sockaddr_in _socket;
		Config &_config;

	public:
		// Constructor and Destructor
		Socket(Config &config, int fd);
		Socket(Config &config);
		Socket &operator=(const Socket& copy);
		~Socket();

		// Data Transmission
		void sendData(Response &response);
		std::string receiveData();

		// Connection Management
		void sendRedirect(const std::string& new_url);
		void redirectToError(int error_code);

		// Getters
		int getSocketFd() const { return _socket_fd; }
};
