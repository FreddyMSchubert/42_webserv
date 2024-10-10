#include "../include/Socket.hpp"
#include <cstring>
#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <sys/types.h>
#include "../include/Logger.hpp"

Socket::Socket() : _socket_pid(-1), _port(0), _address("127.0.0.1")
{
	_socket_pid = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_pid == -1)
		throw std::runtime_error("Socket creation failed");
}

Socket::Socket(int port, std::string ip) : _port(port), _address(ip)
{
	_socket_pid = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_pid == -1)
		throw std::runtime_error("Socket creation failed");
}

void Socket::Run()
{
	Logger::Log(LogLevel::INFO, "Running Socket...");
	try
	{
		_connect();
	}
	catch(const std::exception &e)
	{
		throw std::runtime_error(e.what());
	}
	_setNonBlocking(_socket_pid);

	fd_set read_fds;
	int max_fd = _socket_pid;

	while (42)
	{
		FD_ZERO(&read_fds);
		FD_SET(_socket_pid, &read_fds);

		for (int client_fd : _clients)
		{
			FD_SET(client_fd, &read_fds);
			if (client_fd > max_fd)
				max_fd = client_fd;
		}

		int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
		if (activity < 0 && errno != EINTR)
			throw std::runtime_error("select error");

		if (FD_ISSET(_socket_pid, &read_fds))
		{
			int new_socket = accept(_socket_pid, nullptr, nullptr);
			if (new_socket < 0)
				throw std::runtime_error("Failed to accept connection");
			_setNonBlocking(new_socket);
			_clients.push_back(new_socket);
			Logger::Log(LogLevel::INFO, "New connection accepted!");
		}

		for (auto it = _clients.begin(); it != _clients.end(); )
		{
			int client_fd = *it;
			if (FD_ISSET(client_fd, &read_fds))
			{
				char buffer[1024];
				ssize_t received = recv(client_fd, buffer, sizeof(buffer), 0);
				if (received <= 0)
				{
					// XXX: this is not working properly
					closeSocket(client_fd);
					it = _clients.erase(it);
				}
				else
				{
					
					// Request request(std::string(buffer, received)); // XXX: i think in my version parsing is not fully working yet
					Logger::Log(LogLevel::INFO, "Received data!");
					// request.Run();
					try
					{
						std::string resonse_Str = "HTTP/1.1 200 OK\r\n"
							"Content-Type: text/html\r\n"
							"Content-Length: 20\r\n"
							"\r\n"
							"<h1>Hello 42</h1>";
						// Response res (resonse_Str);
						sendData(resonse_Str, client_fd);
						closeSocket(client_fd);
						Logger::Log(LogLevel::INFO, "Data sent!");
					}
					catch (std::exception &e)
					{
						Logger::Log(LogLevel::ERROR, std::string("Failed to send data: ") + e.what());
					}
					it = _clients.erase(it);
				}
			}
			else ++it;
		}
	}
}

void Socket::sendData(Response &response, int client_fd)
{
	ssize_t sent = send(client_fd, response.getRawPacket().c_str(), response.getRawPacket().length(), 0);
	if (sent < 0)
		throw std::runtime_error("Failed to send data");
	else
		Logger::Log(LogLevel::INFO, "Data sent!");
}

void Socket::sendData(const std::string &data, int socket_fd)
{
	ssize_t sent = send(socket_fd, data.c_str(), data.length(), 0);
	if (sent < 0)
		throw std::runtime_error("Failed to send data");
	else
	 	Logger::Log(LogLevel::INFO, "Data sent!");
}

void Socket::_connect()
{
	std::memset(&_socket, 0, sizeof(_socket));
	_socket.sin_family = AF_INET;
	_socket.sin_port = htons(_port);
	_socket.sin_addr.s_addr = INADDR_ANY; // TODO: change this to given ip (inet_addr(_address.c_str()))
	
	if (bind(_socket_pid, (struct sockaddr *)&_socket, sizeof(_socket)) == -1)
	{
		close(_socket_pid);
		std::string msg("Failed to bind socket: ");
		std::string error(strerror(errno));
		throw std::runtime_error(msg + error);
	}

	if (listen(_socket_pid, 10) == -1)
	{
		close(_socket_pid);
		throw std::runtime_error("Failed to listen on socket");
	}

	Logger::Log(LogLevel::INFO, "Socket connected!");

}

void Socket::_close()
{
	Logger::Log(LogLevel::INFO, "Closing socket...");
	if (_socket_pid != -1)
	{
		close(_socket_pid);
		_socket_pid = -1;
	}
	for (auto it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(*it);
	}
	_clients.clear();
	Logger::Log(LogLevel::INFO, "Socket closed!");
}

void Socket::closeSocket(int socket)
{
	Logger::Log(LogLevel::INFO, "Trying to close socket: " + std::to_string(socket));
	auto it = std::find(_clients.begin(), _clients.end(), socket);
	if (it != _clients.end())
	{
		close(socket);
		Logger::Log(LogLevel::INFO, "Socket closed: " + std::to_string(socket));
	}
	else
		Logger::Log(LogLevel::WARNING, "Attempted to close non-existent socket: " + std::to_string(socket));
}

void Socket::_setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		throw std::runtime_error("Failed to get socket flags");
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		throw std::runtime_error("Failed to set non-blocking mode");
	}
}

Socket::~Socket()
{
	_close();
	Logger::Log(LogLevel::INFO, "Socket closed!");
}