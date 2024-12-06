#include "Server.hpp"
#include "Socket.hpp"

//Setup the listening socket and push it to the vector of sockets
Server::Server(Config &config) : _config(config), _listening_socket{-1, -1, e_socket_state::CLOSE, Socket(config), std::stringstream()}
{
	Socket listeningSocket(config);
	listeningSocket.connectSocket();
	listeningSocket.setNonBlockingSocket(listeningSocket.getSocketFd());

	_listening_socket.fd = listeningSocket.getSocketFd();
	_listening_socket.socket = listeningSocket;

	Logger::Log(LogLevel::INFO, "Server listening on " + _config.getHost() + ":" + std::to_string(_config.getPort()) + ".");
	Run();
}

void Server::updatePoll()
{
	// Fill the vector of pollfd with the sockets
	std::vector<struct pollfd> fds;

	//listening socket
	struct pollfd listen_pfd;
	listen_pfd.fd = _listening_socket.socket.getSocketFd();
	listen_pfd.events = POLLIN;  // Only needs to accept connections therefore always POLLIN
	fds.push_back(listen_pfd);

	//client sockets
	for (size_t i = 0; i < _sockets.size(); ++i)
	{
		struct pollfd pfd;
		pfd.fd = _sockets[i].socket.getSocketFd();
		
		if (_sockets[i].state == e_socket_state::READ)
			pfd.events = POLLIN;
		else if (_sockets[i].state == e_socket_state::WRITE)
			pfd.events = POLLOUT;
		else
			pfd.events = POLLIN | POLLOUT; //watch both if unsure
			
		fds.push_back(pfd);
	}

	//poll trough every socket and check if there is any data to read
	int ret = poll(fds.data(), fds.size(), -1);
	if (ret < 0)
	{
		Logger::Log(LogLevel::ERROR, "Poll error: " + std::string(strerror(errno)) + " -> means that there is no data to read.");
		return;
	}

	// Handle events
	for (size_t i = 0; i < fds.size(); ++i)
	{
		if (fds[i].revents & POLLIN)
		{
			// Listening socket
			if (i == 0)
			{
				// Handle new connection directly here
				struct sockaddr_in client_addr;
				socklen_t addrlen = sizeof(client_addr);
				int client_fd = accept(_listening_socket.socket.getSocketFd(), (struct sockaddr*)&client_addr, &addrlen);
				if (client_fd < 0)
				{
					Logger::Log(LogLevel::ERROR, "Failed to accept new client connection: " + std::string(strerror(errno)));
					continue;
				}

				_listening_socket.socket.setNonBlockingSocket(client_fd);

				Socket clientSocket(_config);
				clientSocket.setSocketFd(client_fd);
				_sockets.emplace_back(client_fd, -1, e_socket_state::READ, clientSocket, std::stringstream());
			}
			else // Client socket
			{
				// Read data from client in buffer
				_sockets[i].state = e_socket_state::READ;
				char buffer[_config.getClientMaxBodySize()];
				ssize_t bytes = recv(_sockets[i].fd, buffer, sizeof(buffer), 0);

				// Handle errors
				if (bytes <= 0 || bytes > _config.getClientMaxBodySize())
				{
					if (bytes <= 0)
						Logger::Log(LogLevel::INFO, "Connection closed by client");
					else if (bytes > _config.getClientMaxBodySize()) //as far as i know if that gets triggered we should display a 413 http error
						Logger::Log(LogLevel::ERROR, "Client sent too much data, closing socket");
					_sockets[i].state = e_socket_state::CLOSE;
					_sockets[i].socket.closeSocket(_sockets[i].fd);
					_sockets.erase(_sockets.begin() + i);
					--i;
				}
				// Write data to buffer
				_sockets[i].buffer.write(buffer, bytes);
			}
		}
		else if (fds[i].revents & POLLOUT)
		{
			_sockets[i].state = e_socket_state::WRITE;
		}
		else if (fds[i].revents & (POLLHUP | POLLERR))
		{
			if (fds[i].revents & POLLHUP)
				Logger::Log(LogLevel::INFO, "Client disconnected");
			else
				Logger::Log(LogLevel::ERROR, "Error occurred on client socket: " + std::string(strerror(errno)));
			_sockets[i].state = e_socket_state::CLOSE;
			_sockets[i].socket.closeSocket(_sockets[i].fd);
			_sockets.erase(_sockets.begin() + i);
			--i;
		}
	}
	// Events to watch for
	// POLLIN    // Ready to read (incoming requests/data)
	// POLLOUT   // Ready to write (sending responses)
	// POLLHUP   // Client disconnected
	// POLLERR   // Error occurred
}

e_complete_data Server::isDataComplete(t_socket_data &socket) // used an enum
{
	std::string data = socket.buffer.str();
	std::regex pattern(HTTP_MIN_HEADER_PATTERN);

	// Case 1: Check if data is empty or something else than POST/DELETE/GET -> Incomplete
	if (data.empty() || (data.find("POST") == std::string::npos && data.find("DELETE") == std::string::npos && data.find("GET") == std::string::npos))
	{
		socket.state = e_socket_state::READ;
		return e_complete_data::INCOMPLETE;
	}
	
	// Case 2: Check for chunked transfer -> Chunked finished or unfinished
	if (data.find("Transfer-Encoding: chunked") != std::string::npos && std::regex_match(data, pattern))
	{
		if (data.find("0\r\n\r\n") != std::string::npos)
		{
			socket.state = e_socket_state::WRITE;
			return e_complete_data::CHUNKED_FINISHED;
		}
		socket.state = e_socket_state::READ;
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
				socket.state = e_socket_state::WRITE;
				return e_complete_data::COMPLETE;
			}
		}
	}
	socket.state = e_socket_state::READ;
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
	while (true)
	{
		// Monitor sockets
		updatePoll();

		// Process received data
		for (size_t i = 0; i < _sockets.size(); ++i)
		{
			e_complete_data status = isDataComplete(_sockets[i]);
			
			switch (status)
			{
				case e_complete_data::COMPLETE:
				case e_complete_data::CHUNKED_FINISHED:
					// handleRequest(_sockets[i]);  // Separate function for request handling or whatever (dunno if thats correct or where we get the response)
					break;
					
				case e_complete_data::CHUNKED_UNFINISHED:
				case e_complete_data::INCOMPLETE:
					continue;  // Wait for more data
					break;
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