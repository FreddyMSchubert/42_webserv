#include "../include/Socket.hpp"
#include <cstring>
#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <sys/types.h>
#include "../include/Logger.hpp"
#include "../include/Utils.hpp"

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
				char buffer[4096];
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
						Request req(std::string(buffer, received));
						if (LOG_INCOMING_PACKETS)
							req.logData();
						std::string path = req.getPath();
						if (path == std::string("/"))
							path = "/index.html";
						std::string response_str;
						try
						{
							response_str = getFileAsString(std::string("./www") + path);
						}
						catch (std::exception &e)
						{
							Logger::Log(LogLevel::ERROR, std::string("Failed to get file: ") + e.what());
							redirectToError(client_fd, 404);
							it = _clients.erase(it);
							continue;
						}
						// Response res (resonse_Str);
						sendData(response_str, client_fd);
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
	sendData(response.getRawPacket(), client_fd);
}

void Socket::sendData(const std::string &data, int socket_fd)
{
	if (LOG_OUTGOING_PACKETS)
		Logger::Log(LogLevel::INFO, "Sending data: " + data);
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

void Socket::redirectToError(int client_fd, int error_code)
{
	int websiteId = rand() % 4;
	std::string website;
	switch(websiteId)
	{
		case 0: website = "https://http.cat/"; break;
		case 1: website = "https://http.dog/"; break;
		case 2: website = "https://http.fish/"; break;
		case 3: website = "https://httpducks.com/"; break;
	}
	website = website + std::to_string(error_code);
	if (websiteId > 0)
		website += ".jpg";

	Logger::Log(LogLevel::INFO, "Redirecting client to " + website + std::to_string(error_code));
	sendRedirect(client_fd, website);
}

void Socket::sendRedirect(int client_fd, const std::string& new_url)
{
	std::string response = "HTTP/1.1 302 Found\r\n";
	response += "Location: " + new_url + "\r\n";
	response += "Content-Length: 0\r\n";
	response += "Connection: close\r\n";
	response += "\r\n"; // End of headers

	sendData(response, client_fd);
	closeSocket(client_fd);
	Logger::Log(LogLevel::INFO, "Redirected client to " + new_url);
}


Socket::~Socket()
{
	_close();
	Logger::Log(LogLevel::INFO, "Socket closed!");
}