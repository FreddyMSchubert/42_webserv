#include "Server.hpp"
#include "Socket.hpp"

//Setup the listening socket and push it to the vector of sockets
Server::Server(Config &config) : _config(config), _listening_socket{config}
{
	Logger::Log(LogLevel::INFO, "Server initialized on " + _config.getHost() + ":" + std::to_string(_config.getPort()));
}

// POLLIN    // Ready to read (incoming requests/data)
// POLLOUT   // Ready to write (sending responses)
// POLLHUP   // Client disconnected
// POLLERR   // Error occurred
void Server::updatePoll()
{
	// Fill the vector of pollfd with the sockets
	std::vector<struct pollfd> fds;

	//client sockets
	for (size_t i = 0; i < _sockets.size(); ++i)
	{
		struct pollfd pfd;
		pfd.fd = _sockets[i].socket.getSocketFd();
		pfd.events = POLLIN | POLLOUT | POLLHUP | POLLERR; //watch all events
		fds.push_back(pfd);
	}

	//listening socket
	struct pollfd listen_pfd;
	listen_pfd.fd = _listening_socket.socket.getSocketFd();
	listen_pfd.events = POLLIN | POLLOUT | POLLERR | POLLHUP;  // Only needs to accept connections therefore always POLLIN
	fds.push_back(listen_pfd);

	//poll trough every socket and check if there is any data to read
	int ret = poll(fds.data(), fds.size(), -1);
	if (ret < 0)
	{
		Logger::Log(LogLevel::ERROR, "Poll error: " + std::string(strerror(errno)) + " -> means that there is no data to read.");
		return;
	}

	// Handle events
	unsigned long fdIndex = -1;
	while (++fdIndex < _sockets.size())
	{
		_sockets[fdIndex].states.read = fds[fdIndex].revents & POLLIN;
		_sockets[fdIndex].states.write = fds[fdIndex].revents & POLLOUT;
		_sockets[fdIndex].states.disconnect = fds[fdIndex].revents & POLLHUP;
		_sockets[fdIndex].states.error = fds[fdIndex].revents & POLLERR;

		if (_sockets[fdIndex].states.read)
		{
			// Read data from client in buffer
			_sockets[fdIndex].buffer << _sockets[fdIndex].socket.receiveData();
		}
		else if (_sockets[fdIndex].states.write)
		{
			// Handle response
			Request req(_sockets[fdIndex].buffer.str());
			Response response(req, _config);
			_sockets[fdIndex].socket.sendData(response);
		}
		else if (_sockets[fdIndex].states.disconnect || _sockets[fdIndex].states.error)
		{
			if (_sockets[fdIndex].states.disconnect)
				Logger::Log(LogLevel::INFO, "Client disconnected");
			else
				Logger::Log(LogLevel::ERROR, "Error occurred on client socket: " + std::string(strerror(errno)));
			_sockets.erase(_sockets.begin() + fdIndex);
		}
	}

	// Listening socket -> Handle new connection directly here
	_listening_socket.states.read = fds[fdIndex].revents & POLLIN;
	_listening_socket.states.write = fds[fdIndex].revents & POLLOUT;
	_listening_socket.states.disconnect = fds[fdIndex].revents & POLLHUP;
	_listening_socket.states.error = fds[fdIndex].revents & POLLERR;

	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	int client_fd = accept(_listening_socket.socket.getSocketFd(), (struct sockaddr*)&client_addr, &addrlen);
	if (client_fd >= 0)
	{
		Socket clientSocket(_config, client_fd);
		_sockets.emplace_back(client_fd, clientSocket);
	}
	else
		Logger::Log(LogLevel::ERROR, "Failed to accept new client connection: " + std::string(strerror(errno)));
}

e_complete_data Server::isDataComplete(t_socket_data &socket) // used an enum
{
	std::string data = socket.buffer.str();
	std::regex pattern(HTTP_MIN_HEADER_PATTERN);

	// Case 0: Ensure buffer is not encumulating the entirety of the lord of the rings trilogy
	if (data.size() > _config.getClientMaxBodySize())
	{
		_sockets.erase(std::remove_if(_sockets.begin(), _sockets.end(), [&socket](const t_socket_data &s) { return s.fd == socket.fd; }), _sockets.end());
		return e_complete_data::INCOMPLETE;
	}

	// Case 1: Check if data is empty or something else than POST/DELETE/GET -> Incomplete
	if (data.empty() || (data.find("POST") == std::string::npos && data.find("DELETE") == std::string::npos && data.find("GET") == std::string::npos))
	{
		return e_complete_data::INCOMPLETE;
	}

	// Case 2: Check for chunked transfer -> Chunked finished or unfinished
	if (data.find("Transfer-Encoding: chunked") != std::string::npos && std::regex_match(data, pattern))
	{
		if (data.find("0\r\n\r\n") != std::string::npos)
		{
			return e_complete_data::CHUNKED_FINISHED;
		}
		return e_complete_data::CHUNKED_UNFINISHED;
	}

	// Case 3: Check Content-Length -> Complete or Incomplete
	if (data.find("Content-Length: ") != std::string::npos && std::regex_match(data, pattern))
	{
		size_t headerEnd = data.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			size_t contentLength = std::stoi(data.substr(data.find("Content-Length: ") + 16, headerEnd - data.find("Content-Length: ") - 16));
			if (data.size() - headerEnd - 4 >= contentLength)
			{
				return e_complete_data::COMPLETE;
			}
		}
	}
	return e_complete_data::INCOMPLETE;
}

// void Server::handleRequest(t_socket_data& socket) //just a placeholder(!) to actually send data back to the client
// {
//     HTTPResponse response = generateResponse(socket.buffer);
//     sendResponse(socket, response);
//     socket.buffer.str("");  // Clear buffer after sending
// }

void Server::Run()
{
	//continuously looping is very resource intensive for that we might use epoll or select
	while (true)
	{
		// Monitor sockets
		updatePoll();

		// Process received data
		for (size_t i = 0; i < _sockets.size(); ++i)
		{
			try
			{
				std::string data = _sockets[i].socket.receiveData();
				e_complete_data status = isDataComplete(_sockets[i]);
				Request req(data);
				Response response(req, _config);
				switch (status)
				{
					case e_complete_data::COMPLETE:
					case e_complete_data::CHUNKED_FINISHED:
						_sockets[i].socket.sendData(response);
						break;
									
					case e_complete_data::CHUNKED_UNFINISHED:
					case e_complete_data::INCOMPLETE:
						continue;  // Wait for more data
						break;
				}
			}
			catch (const std::exception &e)
			{
				std::cout << e.what() << "But the show must go on" << std::endl; //yep testing purposes
			}
		}
	}
}

//How it should work: (i havent written everything but the idea is there)
//1. We create a listening socket (Constructor)
//2. After that we call Run in to start the server (Constructor)
//3. In Run we will loop trough the sockets and check if there is any data to read for the server (updatePoll)
//4. If there is data we will check if the data is complete (and chunked or not and how to handle it) (isDataComplete)
//5. If the data is complete we will send a response back to the client (Run)

//one thing: currently we are not using e_socket_state for anything its just there maybe for debugging but else its useless
