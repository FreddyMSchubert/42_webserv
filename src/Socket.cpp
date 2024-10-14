#include "../include/Socket.hpp"
#include <cstring>
#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <poll.h>
#include "../include/Logger.hpp"
#include "../include/Utils.hpp"

Socket::Socket(t_server_config config) : _socket_pid(-1), config(config)
{
	_socket_pid = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_pid == -1)
		throw std::runtime_error("Socket creation failed");

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
}

std::string Socket::_receiveData(int client_fd, std::vector<int>::iterator &it)
{
	std::string data;
		char buffer[4096];
		ssize_t received;
		
	// TODO: check if this part successfully reads the whole request even if its chunked
	while (true)
	{
		received = recv(client_fd, buffer, sizeof(buffer), 0);
		if (received > 0)
		{
			data.append(buffer, received);
			if (data.find("\r\n\r\n") != std::string::npos)
				break;
		}
		else if (received == 0)
		{
			Logger::Log(LogLevel::INFO, "Connection closed by client");
			closeSocket(client_fd);
			it = _clients.erase(it);
			break;
		}
		else if (received < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break; // No more data to read at this time
			Logger::Log(LogLevel::ERROR, "Failed to receive data: " + std::string(strerror(errno)));
			closeSocket(client_fd);
			it = _clients.erase(it);
			break;
		}
	}

	if (received <= 0)
		throw std::runtime_error("Failed to receive data");
	return data;
}

void Socket::Run()
{
	fd_set read_fds;
	int max_fd = _socket_pid;

	FD_ZERO(&read_fds);
	FD_SET(_socket_pid, &read_fds);

	for (int client_fd : _clients)
	{
		FD_SET(client_fd, &read_fds);
		if (client_fd > max_fd)
			max_fd = client_fd;
	}

	// select timeout
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000; // 100ms

	int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv); // TODO: select is blocking, use poll instead
	// int activity = poll(nullptr, 0, 100); // TODO: select is blocking, use poll instead
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
		if (!FD_ISSET(client_fd, &read_fds))
		{
			++it;
			continue;
		}

		try
		{
			Request req(_receiveData(client_fd, it));
			if (LOG_INCOMING_PACKETS)
				req.logData();
			Logger::Log(LogLevel::INFO, "Received data!");
			std::string response_Str = req.ProcessRequest(config);
			if (response_Str.empty())
			{
				redirectToError(client_fd, 404);
				it = _clients.erase(it);
				continue;
			}
			Response res(response_Str);
			sendData(res, client_fd);
			closeSocket(client_fd);
			Logger::Log(LogLevel::INFO, "Data sent!");
		}
		catch (std::exception &e)
		{
			Logger::Log(LogLevel::ERROR, std::string("Failed to receive or send data: ") + e.what());
		}
		it = _clients.erase(it);
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
	_socket.sin_port = htons(config.port);

	if (inet_pton(AF_INET, config.host.c_str(), &_socket.sin_addr) <= 0)
	{
		close(_socket_pid);
		throw std::runtime_error("Invalid IP address: " + config.host);
	}
	
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

	Logger::Log(LogLevel::INFO, "Redirecting client to " + website + " with code " + std::to_string(error_code) + ".");
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
