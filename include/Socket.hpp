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

class Socket
{
	private:
		int _socket_pid;
		int _port;
		std::string _address;
		struct sockaddr_in _socket;
		std::vector<int> _clients;
		void _connect();
		void _close();
		void _setNonBlocking(int fd);
	public:
		Socket();
		Socket(int port, std::string address);
		Socket(Socket&& other) noexcept = default;
		Socket& operator=(Socket&& other) noexcept = default;
		Socket(const Socket &src);
		Socket &operator=(const Socket &src);
		~Socket();
		void sendData(Response &response, int socket_fd);
		void sendData(const std::string &data, int socket_fd);
		void Run();
		void closeSocket(int socket);
};