#include "Socket.hpp"

Socket::Socket(Config &config) : _socket_fd(-1), _config(config)
{
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1)
		throw std::runtime_error("Socket creation failed");

	Logger::Log(LogLevel::INFO, "Running Socket...");
	try
	{
		connectSocket();
		setNonBlockingSocket(_socket_fd);
	}
	catch(const std::exception &e)
	{
		throw std::runtime_error(e.what());
	}
}

Socket &Socket::operator=(const Socket &copy)
{
	if (this != &copy)
	{
		this->_socket_fd = copy._socket_fd;
		this->_socket = copy._socket;
		this->_clients = copy._clients;
		this->_config = copy._config;
	}
	return (*this);
}

Socket::~Socket()
{
	closeAllSockets();
	Logger::Log(LogLevel::INFO, "Socket closed!");
}

int Socket::addNewSocket()
{
	connectSocket();
	setNonBlockingSocket(_socket_fd);
}

void Socket::connectSocket()
{
	std::memset(&_socket, 0, sizeof(_socket));
	_socket.sin_family = AF_INET;
	_socket.sin_port = htons(_config.getPort());

	if (inet_pton(AF_INET, _config.getHost().c_str(), &_socket.sin_addr) <= 0)
	{
		close(_socket_fd);
		throw std::runtime_error("Invalid IP address: " + _config.getHost());
	}
	
	if (bind(_socket_fd, (struct sockaddr *)&_socket, sizeof(_socket)) == -1)
	{
		close(_socket_fd);
		throw std::runtime_error("Failed to bind socket");
	}

	if (listen(_socket_fd, 10) == -1)
	{
		close(_socket_fd);
		throw std::runtime_error("Failed to listen on socket");
	}

	Logger::Log(LogLevel::INFO, "Socket connected!");
}

void Socket::setNonBlockingSocket(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Failed to get socket flags");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set non-blocking mode");
}

void Socket::sendData(Response &response, int client_fd)
{
	sendData(response.getRawPacket(), client_fd);
}

void Socket::sendData(const std::string &data, int socket_fd)
{
	#if LOG_OUTGOING_PACKETS
		Logger::Log(LogLevel::INFO, "Sending data: " + data);
	#endif
	ssize_t sent = send(socket_fd, data.c_str(), data.length(), 0);
	if (sent < 0)
		throw std::runtime_error("Failed to send data");
	else
		Logger::Log(LogLevel::INFO, "Data sent!");
}

// TODO: handle protocols other then HTTP (probably not) that dont send \r\n\r\n in the end
std::string Socket::receiveData(int client_fd)
{
	std::string data;
	char buffer[4096];
	ssize_t received;
	int max_loops = 1000;

	while (--max_loops)
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
			break;
		}
		else if (received < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			Logger::Log(LogLevel::ERROR, "Failed to receive data: " + std::string(strerror(errno)));
			closeSocket(client_fd);
			break;
		}
	}

	if (data.empty())
		Logger::Log(LogLevel::WARNING, "Tried to receive data but received nothing");
	return data;
}

void Socket::closeSocket(int socket)
{
	Logger::Log(LogLevel::INFO, "Trying to close socket: " + std::to_string(socket));
	int i = 0;

	for (auto &client : _clients)
	{
		if (client.fd == socket)
		{
			close(client.fd);
			_clients.erase(_clients.begin() + i);
			Logger::Log(LogLevel::INFO, "Socket closed: " + std::to_string(socket));
			return;
		}
		++i;
	}

	Logger::Log(LogLevel::WARNING, "Attempted to close non-existent socket: " + std::to_string(socket));
}

void Socket::closeAllSockets()
{
	Logger::Log(LogLevel::INFO, "Closing socket...");
	for (auto &client : _clients)
		close(client.fd);
	_socket_fd = -1;
	_clients.clear();
	Logger::Log(LogLevel::INFO, "Socket closed!");
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
