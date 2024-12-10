#include "Socket.hpp"

// Listening socket constructor
Socket::Socket(Config &config) : _config(config)
{
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1)
		throw std::runtime_error("Socket creation failed");

	Logger::Log(LogLevel::INFO, "Running Listening Socket on fd " + std::to_string(_socket_fd) + "...");
	try
	{
		std::memset(&_socket, 0, sizeof(_socket));
		_socket.sin_family = AF_INET;
		_socket.sin_port = htons(_config.getPort());

		if (inet_pton(AF_INET, _config.getHost().c_str(), &_socket.sin_addr) <= 0)
		{
			close(_socket_fd);
			throw std::runtime_error("Invalid IP address: " + _config.getHost());
		}

		int opt = 1;
		if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}
		
		if (bind(_socket_fd, (struct sockaddr *)&_socket, sizeof(_socket)) < 0)
		{	
			close(_socket_fd);
			throw std::runtime_error("Failed to bind socket");
		}

		if (listen(_socket_fd, 10) == -1)
		{
			close(_socket_fd);
			throw std::runtime_error("Failed to listen on socket");
		}

		Logger::Log(LogLevel::INFO, "Socket " + std::to_string(_socket_fd) + " connected!");

		// set non-blocking
		int flags = fcntl(_socket_fd, F_GETFL, 0);
		if (flags == -1)
			throw std::runtime_error("Failed to get socket flags");
		if (fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
			throw std::runtime_error("Failed to set non-blocking mode");
	}
	catch(const std::exception &e)
	{
		std::cout << "Hi there" << std::endl;
		throw std::runtime_error(e.what());
	}
}
// Client socket constructor
Socket::Socket(Config &config, int fd) : _config(config)
{
	_socket_fd = fd;
	Logger::Log(LogLevel::INFO, "Running Client Connection Socket...");
}

Socket::Socket(Socket&& other) noexcept
	: _socket_fd(other._socket_fd),
	_socket(other._socket),
	_config(other._config)
{
	other._socket_fd = -1; // Mark as moved; prevents double close
}
Socket& Socket::operator=(Socket&& other) noexcept
{
	if (this == &other)
		return *this;
	if (_socket_fd >= 0)
		close(_socket_fd);
	_socket_fd = other._socket_fd;
	_socket = other._socket;
	other._socket_fd = -1; // Mark as moved; prevents double close
	return *this;
}
Socket::~Socket()
{
	if (_socket_fd >= 0)
	{
		close(_socket_fd);
		Logger::Log(LogLevel::INFO, "Socket " + std::to_string(_socket_fd) + " closed");
	}
}

void Socket::sendData(Response &response)
{
	#if LOG_OUTGOING_PACKETS
		Logger::Log(LogLevel::INFO, "Sending data: " + data);
	#endif
	ssize_t sent = send(_socket_fd, response.getRawPacket().c_str(), response.getRawPacket().length(), 0);
	if (sent < 0)
		throw std::runtime_error("Failed to send data");
	else
		Logger::Log(LogLevel::INFO, "Data sent!");
}

// TODO: handle protocols other then HTTP (probably not) that dont send \r\n\r\n in the end
// Socket.cpp

std::string Socket::receiveData()
{
	std::string data;
	char buffer[_config.getClientMaxBodySize() + 1];
	ssize_t received;

	received = recv(_socket_fd, buffer, sizeof(buffer), 0);
	if (received > 0)
	{
		data.append(buffer, received);
		// Process data...
	}
	else if (received < 0)
	{
		Logger::Log(LogLevel::ERROR, "Failed to receive data: " + std::string(strerror(errno)));
		throw std::runtime_error("Failed to receive data");
	}
	else // received == 0
	{
		// Connection closed by client
		throw std::runtime_error("Client disconnected");
	}

	if (data.empty())
	{
		Logger::Log(LogLevel::WARNING, "Received no data");
	}
	return data;
}

void Socket::sendRedirect(const std::string& new_url)
{
	std::string response = "HTTP/1.1 302 Found\r\n";
	response += "Location: " + new_url + "\r\n";
	response += "Content-Length: 0\r\n";
	response += "Connection: close\r\n";
	response += "\r\n"; // End of headers

	Request request(response);
	Response responsePacket(request, _config);
	sendData(responsePacket);
	Logger::Log(LogLevel::INFO, "Redirected client to " + new_url);
}

void Socket::redirectToError(int error_code)
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
	sendRedirect(website);
}
