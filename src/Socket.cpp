#include "Socket.hpp"

// Listening socket constructor
Socket::Socket(Config &config) : _config(config)
{
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1)
		throw std::runtime_error("Socket creation failed");

	Logger::Log(LogLevel::INFO, config.getServerId(), "Running Listening Socket on fd " + std::to_string(_socket_fd) + "...");
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
			throw std::runtime_error("Failed to bind socket" + std::to_string(_socket_fd));
		}

		if (listen(_socket_fd, 10) == -1)
		{
			close(_socket_fd);
			throw std::runtime_error("Failed to listen on socket");
		}

		Logger::Log(LogLevel::INFO, config.getServerId(), "Socket " + std::to_string(_socket_fd) + " connected!");
		setNonBlocking();
	}
	catch(const std::exception &e)
	{
		throw std::runtime_error(e.what());
	}
}
// Client socket constructor
Socket::Socket(Config &config, int fd) : _config(config)
{
	_socket_fd = fd;
	setNonBlocking();
	Logger::Log(LogLevel::INFO, config.getServerId(), "Running Client Connection Socket " + std::to_string(_socket_fd) + "...");
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
		Logger::Log(LogLevel::INFO, _config.getServerId(), "Socket " + std::to_string(_socket_fd) + " closed");
	}
}

void Socket::sendData(const std::string & data)
{
	#if LOG_OUTGOING_PACKETS
		Logger::Log(LogLevel::INFO, _config.getServerId(), "Sending data: " + data);
	#endif
	ssize_t sent = send(_socket_fd, data.c_str(), data.length(), 0);
	if (sent < 0)
		throw std::runtime_error("Failed to send data");
	else
		Logger::Log(LogLevel::INFO, _config.getServerId(), "Data sent!");
}

std::string Socket::receiveData()
{
	std::string data;
	int size = std::min(1048576, static_cast<int>(_config.getMaxPackageSize() + 1));
	char buffer[size];
	ssize_t received;

	received = recv(_socket_fd, buffer, sizeof(buffer), 0);
	if (received > 0)
		data.append(buffer, received);
	else if (received <= 0)
	{
		Logger::Log(LogLevel::WARNING, _config.getServerId(), "Failed to receive data");
		return "";
	}

	return data;
}

void Socket::sendRedirect(const std::string& new_url, int status_code)
{
	std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " + Packet::getStatusMessage(status_code) + "\r\n";
	response += "Location: " + new_url + "\r\n";
	response += "Content-Length: 0\r\n";
	response += "Connection: close\r\n";
	response += "\r\n"; // End of headers

	Request request(response);
	Response responsePacket(request, _config);
	sendData(responsePacket.getRawPacket());
	Logger::Log(LogLevel::INFO, _config.getServerId(), "Redirected client to " + new_url);
}
void Socket::redirectToOtherResource(FilePath path, int status_code)
{
	std::string body;
	try
	{
		body = path.getFileContents();
	}
	catch (const std::exception &e)
	{
		Logger::Log(LogLevel::ERROR, _config.getServerId(), "Failed to read file: " + path.asFilePath());
		redirectToError(Status::NotFound);
		return;
	}
	std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " + Packet::getStatusMessage(status_code) + "\r\n";
	response += "Content-Type: " + path.getMimeType() + "\r\n";
	response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += body;

	sendData(response);
	Logger::Log(LogLevel::INFO, _config.getServerId(), "Redirected client to other resource: " + path.asUrl());
}
void Socket::redirectToError(Status error_code)
{
	int errorCodeInt = static_cast<int>(error_code);

	// 1. Custom error pages
	std::map<int, FilePath> errorPages = _config.getErrorPages();
	if (errorPages.find(errorCodeInt) != errorPages.end())
	{
		FilePath errorPage = errorPages.at(errorCodeInt);
		std::string body;
		try {
			body = errorPage.getFileContents();
		} catch (const std::exception &e) {
			Logger::Log(LogLevel::ERROR, _config.getServerId(), "Failed to read custom error page: " + errorPage.asUrl());
			return;
			// redirectToError(Status::NotFound);
		}
		std::string response = "HTTP/1.1 " + std::to_string(errorCodeInt) + " " + Packet::getStatusMessage(errorCodeInt) + "\r\n";
		response += "Content-Type: text/html\r\n";
		response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
		response += "Connection: close\r\n";
		response += "\r\n";
		response += body;

		sendData(response);
		Logger::Log(LogLevel::INFO, _config.getServerId(), "Redirected client to custom error page with code " + std::to_string(errorCodeInt) + ".");
		return;
	}

	// 2. Noel's memery
	bool noel = true;
	std::string title;
	std::string image;
	std::string text;
	std::string positioning;
	std::string status_line;
	switch (errorCodeInt)
	{
		case 302:
			title = "302 - Found";
			image = "https://i.imgflip.com/9dx1g6.jpg";
			text = "302 - Found. But u know what we didnt find? Your father.";
			positioning = "";
			status_line = "HTTP/1.1 302 Found\r\n";
			break;
		case 400:
			title = "400 - Bad Request";
			image = "https://i.imgflip.com/9dx3dr.jpg";
			text = "400 - Bad Request. Listen to your mother and stop making bad requests!";
			positioning = "";
			status_line = "HTTP/1.1 400 Bad Request\r\n";
			break;
		case 404:
			title = "404 - Not Found";
			image = "https://img.sparknews.funkemedien.de/214885251/214885251_1532012732_v16_9_1600.webp";
			text = "404 - Not Found";
			positioning = "position: absolute; left: 0%;";
			status_line = "HTTP/1.1 404 Not Found\r\n";
			break;
		case 413:
			title = "413 - Payload Too Large";
			image = "https://i0.wp.com/worleygig.com/wp-content/uploads/2014/05/p1030261-e1401153271349.jpg";
			text = "413 - Payload Too Large. Thats what she said!";
			status_line = "HTTP/1.1 413 Payload Too Large\r\n";
			positioning = "";
			break;
		case 501:
			title = "501 - Not Implemented";
			image = "https://i.imgflip.com/9dx6ze.jpg";
			text = "501 - Not Implemented. You know what else is not implemented? Your father.";
			positioning = "";
			status_line = "HTTP/1.1 501 Not Implemented\r\n";
			break;
		default:
			noel = false;
			break;
	}

	if (noel)
	{
		std::string template_data = getFileData("templates/error_page.html");
		size_t title_pos = template_data.find("{title}");
		if (title_pos != std::string::npos)
			template_data.replace(title_pos, 7, title);
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

		sendData(response);
		Logger::Log(LogLevel::INFO, _config.getServerId(), "Redirected client to custom error page with code " + std::to_string(errorCodeInt) + ".");
		return;
	}

	// 3. Generic cat/dog/fish/duck images
	int websiteId = rand() % 4;
	std::string website;
	switch(websiteId)
	{
		case 0: website = "https://http.cat/"; break;
		case 1: website = "https://http.dog/"; break;
		case 2: website = "https://http.fish/"; break;
		case 3: website = "https://httpducks.com/"; break;
	}
	website = website + std::to_string(errorCodeInt);
	if (websiteId > 0)
		website += ".jpg";

	Logger::Log(LogLevel::INFO, _config.getServerId(), "Redirecting client to " + website + " with code " + std::to_string(errorCodeInt) + ".");
	sendRedirect(website, errorCodeInt);
}

void Socket::setNonBlocking()
{
	int flags = fcntl(_socket_fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Failed to get socket " + std::to_string(_socket_fd) + " flags");
	if (fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set non-blocking mode on socket " + std::to_string(_socket_fd));
}
