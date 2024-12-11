#include "Socket.hpp"

Socket::Socket(Config &config) : _socket_pid(-1), _config(config)
{
	_socket_pid = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_pid == -1)
		throw std::runtime_error("Socket creation failed");

	Logger::Log(LogLevel::INFO, "Running Socket...");
	try
	{
		_connect();
		_setNonBlocking(_socket_pid);
	}
	catch(const std::exception &e)
	{
		throw std::runtime_error(e.what());
	}
}

// TODO: handle protocols other then HTTP (probably not) that dont send \r\n\r\n in the end
std::string Socket::_receiveData(int client_fd)
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

void Socket::Run()
{
	_clients.push_back({_socket_pid, POLLIN, 0});

	int activity = poll(_clients.data(), _clients.size(), 100);
	if (activity < 0 && errno != EINTR)
		throw std::runtime_error("poll error");

	for (auto &client : _clients)
	{

		if (client.revents & (POLLHUP | POLLERR | POLLNVAL))
		{
			Logger::Log(LogLevel::INFO, "Client disconnected!");
			closeSocket(client.fd);
			break;
		}

		if (client.revents & POLLIN && client.fd == _socket_pid)
		{
			int new_socket = accept(_socket_pid, nullptr, nullptr);
			if (new_socket < 0)
				Logger::Log(LogLevel::ERROR, "Failed to accept new connection!");
			else
			{
				try
				{
					_setNonBlocking(new_socket);
				}
				catch(const std::exception &e)
				{
					Logger::Log(LogLevel::ERROR, e.what());
					closeSocket(new_socket);
					break;
				}
				_clients.push_back({new_socket, POLLIN, 0});
				Logger::Log(LogLevel::INFO, "New connection accepted!");
			}
			continue;
		}

		if (client.revents & POLLIN && client.fd != _socket_pid)
		{
			std::string data = _receiveData(client.fd);
			if (data.empty())
			{
				Logger::Log(LogLevel::INFO, "Client sent empty data, closing connection!");
				closeSocket(client.fd);
				break;
			}

			Logger::Log(LogLevel::INFO, "Data received from client!");
			Request req(data);
			#if LOG_INCOMING_PACKETS
				req.logData();
			#endif

			Response response = Response(req, _config);
			sendData(response, client.fd);

			closeSocket(client.fd);
			// redirectToError(client.fd, 413);
			Logger::Log(LogLevel::INFO, "Response sent and connection closed!");
			break;
		}
	}
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

void Socket::_connect()
{
	std::memset(&_socket, 0, sizeof(_socket));
	_socket.sin_family = AF_INET;
	_socket.sin_port = htons(_config.getPort());

	if (inet_pton(AF_INET, _config.getHost().c_str(), &_socket.sin_addr) <= 0)
	{
		close(_socket_pid);
		throw std::runtime_error("Invalid IP address: " + _config.getHost());
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
	for (auto &client : _clients)
		close(client.fd);
	_socket_pid = -1;
	_clients.clear();
	Logger::Log(LogLevel::INFO, "Socket closed!");
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

void Socket::_setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Failed to get socket flags");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set non-blocking mode");
}

void Socket::redirectToError(int client_fd, int error_code)
{
	// 1. Noel's memery
	bool noel = true;
	std::string image;
	std::string text;
	std::string positioning;
	std::string status_line;
	switch (error_code)
	{
		case 404:
			image = "https://img.sparknews.funkemedien.de/214885251/214885251_1532012732_v16_9_1600.webp";
			text = "404 - Not Found";
			positioning = "position: absolute; left: 0%;";
			status_line = "HTTP/1.1 404 Not Found\r\n";
			break;
		case 413:
			image = "https://i0.wp.com/worleygig.com/wp-content/uploads/2014/05/p1030261-e1401153271349.jpg";
			text = "413 - Payload Too Large. Thats what she said!";
			status_line = "HTTP/1.1 413 Payload Too Large\r\n";
			positioning = "";
			break;
		default:
			noel = false;
			break;
	}

	if (noel)
	{
		std::string template_data = getFileData("templates/error_page.html");
		size_t img_pos = template_data.find("{img}");
		if (img_pos != std::string::npos)
			template_data.replace(img_pos, 5, image);
		size_t text_pos = template_data.find("{text}");
		if (text_pos != std::string::npos)
			template_data.replace(text_pos, 6, text);
		size_t pos_pos = template_data.find("{pos}");
		if (pos_pos != std::string::npos)
			template_data.replace(pos_pos, 13, positioning);
		
		std::string response = status_line;
		response += "Content-Type: text/html\r\n";
		response += "Content-Length: " + std::to_string(template_data.length()) + "\r\n";
		response += "Connection: close\r\n";
		response += "\r\n"; // End of headers
		response += template_data;

		sendData(response, client_fd);
		closeSocket(client_fd);
		Logger::Log(LogLevel::INFO, "Redirected client to custom error page with code " + std::to_string(error_code) + ".");
		return;
	}

	// 2. Generic cat/dog/fish/duck images
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
