#include "Server.hpp"
#include "Socket.hpp"

//Setup the listening socket and push it to the vector of sockets
Server::Server(Config &config) : _config(config), _listening_socket{config}
{
	Logger::Log(LogLevel::INFO, config.getServerId(), "Server initialized on " + _config.getHost() + ":" + std::to_string(_config.getPort()));
}

// POLLIN    // Ready to read (incoming requests/data)
// POLLOUT   // Ready to write (sending responses)
// POLLHUP   // Client disconnected
// POLLERR   // Error occurred
void Server::updatePoll()
{
	// Create pollfds for poll call
	std::vector<struct pollfd> fds;
	for (size_t i = 0; i < _sockets.size(); ++i)
	{
		struct pollfd pfd;
		pfd.fd = _sockets[i].socket.getSocketFd();
		pfd.events = POLLIN | POLLOUT | POLLHUP | POLLERR;
		fds.push_back(pfd);
	}
	struct pollfd listen_pfd;
	listen_pfd.fd = _listening_socket.socket.getSocketFd();
	listen_pfd.events = POLLIN | POLLOUT | POLLERR | POLLHUP;
	fds.push_back(listen_pfd);

	// poll
	int ret = poll(fds.data(), fds.size(), 0);
	if (ret < 0)
	{
		Logger::Log(LogLevel::ERROR, _config.getServerId(), "Poll error: " + std::string(strerror(errno)) + " -> means that there is no data to read.");
		return;
	}

	// Read out poll data
	_listening_socket.states.read = fds[fds.size() - 1].revents & POLLIN;
	_listening_socket.states.write = fds[fds.size() - 1].revents & POLLOUT;
	_listening_socket.states.disconnect = fds[fds.size() - 1].revents & POLLHUP;
	_listening_socket.states.error = fds[fds.size() - 1].revents & POLLERR;
	for (size_t i = 0; i < fds.size() - 1; i++)
	{
		_sockets[i].states.read = fds[i].revents & POLLIN;
		_sockets[i].states.write = fds[i].revents & POLLOUT;
		_sockets[i].states.disconnect = fds[i].revents & POLLHUP;
		_sockets[i].states.error = fds[i].revents & POLLERR;
	}
}

bool Server::isDataComplete(t_socket_data &socket)
{
	std::string data = socket.buffer.str();
	size_t headerEnd = data.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;

	// Chunked transfer encoding
	if (data.find("Transfer-Encoding: chunked") != std::string::npos)
		return data.find("0\r\n\r\n") != std::string::npos;

	// Normal packet
	if (data.find("Content-Length: ") != std::string::npos)
	{
		size_t headerEnd = data.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			size_t contentLength = std::stoi(data.substr(data.find("Content-Length: ") + 16, headerEnd - data.find("Content-Length: ") - 16));
			if (data.size() - headerEnd - 4 > contentLength)
				throw std::runtime_error("Client data size exceeded declared content length");
			else
				return data.size() - headerEnd - 4 == contentLength;
		}
	}

	return true;
}

void Server::acceptNewConnections()
{
	if (!_listening_socket.states.read)
		return;
	int max_iterations = 10;
	while (max_iterations > 0)
	{
		struct sockaddr_in client_addr;
		socklen_t addrlen = sizeof(struct sockaddr_in);
		int client_fd = accept(_listening_socket.socket.getSocketFd(), (struct sockaddr*)&client_addr, &addrlen);
		if (client_fd >= 0)
		{
			Logger::Log(LogLevel::INFO, _config.getServerId(), "New client connected");
			_sockets.emplace_back(client_fd, Socket(_config, client_fd));
			max_iterations--;
		}
		else
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK) // i believe accept is not an i/o operation
				Logger::Log(LogLevel::ERROR, _config.getServerId(), "Accept error: " + std::string(strerror(errno)));
			break;
		}
	}
}

void Server::handleExistingConnections()
{
	for (int i = (int)_sockets.size() - 1; i >= 0; i--)
	{
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
		std::chrono::time_point<std::chrono::steady_clock> last_activity = _sockets[i].last_activity;
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_activity).count() > _config.getClientTimeout())
		{
			Logger::Log(LogLevel::INFO, _config.getServerId(), "Client " + std::to_string(_sockets[i].fd) + " timed out");
			_sockets.erase(_sockets.begin() + i);
			continue;
		}
		if (_sockets[i].states.read)
		{
			Logger::Log(LogLevel::INFO, _config.getServerId(), "Reading data from client");
			try
			{
				_sockets[i].buffer << _sockets[i].socket.receiveData();
				if (_sockets[i].buffer.str().size() > _config.getMaxPackageSize())
				{
					Logger::Log(LogLevel::ERROR, _config.getServerId(), "Client data size exceeded maximum body size");
					_sockets.erase(_sockets.begin() + i);
					continue;
				}
				_sockets[i].last_activity = std::chrono::steady_clock::now();
			}
			catch(const std::runtime_error &e)
			{
				Logger::Log(LogLevel::WARNING, _config.getServerId(), "Erroneous packet data: " + std::string(e.what()));
				_sockets.erase(_sockets.begin() + i);
				continue;
			}
		}
		if (_sockets[i].states.write)
		{
			bool dataComplete;
			try
			{
				dataComplete = isDataComplete(_sockets[i]);
			}
			catch (const std::runtime_error &e)
			{
				Logger::Log(LogLevel::WARNING, _config.getServerId(), "Erroneous packet data: " + std::string(e.what()));
				_sockets.erase(_sockets.begin() + i);
				continue;
			}
			if (dataComplete)
			{
				Logger::Log(LogLevel::INFO, _config.getServerId(), "Sending response to client");
				Request req(_sockets[i].buffer.str());
				Response res(req, _config);
				_sockets[i].socket.sendData(res);
				// _sockets[i].socket.redirectToError(501); //testing for error pages
				_sockets.erase(_sockets.begin() + i);
				continue;
			}
		}
		if (_sockets[i].states.disconnect || _sockets[i].states.error)
		{
			if (_sockets[i].states.disconnect)
				Logger::Log(LogLevel::INFO, _config.getServerId(), "Client disconnected");
			else
				Logger::Log(LogLevel::ERROR, _config.getServerId(), "Error occurred on client socket: " + std::string(strerror(errno)));
			_sockets.erase(_sockets.begin() + i);
			continue;
		}
	}
}

void Server::Run()
{
	updatePoll();
	acceptNewConnections();
	handleExistingConnections();
}
