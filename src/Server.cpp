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

// So generally what happens here is this: we loop trough the fds which are structs of pollfd and check:
//if the event we receive is POLLIN (so if there is any readable data recieved)
//i will check if we recieved it on the listening socket if thats the case i will make a new socket for this client and
//if its not a new connection we go to the else and there we use recv is giving us how long the message that was written in the buffer
//and if its zero we close the socket and if there is something we write it to the struct to use it later on.
void Server::updatePoll()
{
	std::vector<struct pollfd> fds;
	for (size_t i = 0; i < _sockets.size(); ++i)
	{
		struct pollfd pfd;
		pfd.fd = _sockets[i].socket.getSocketFd();
		pfd.events = POLLIN;
		fds.push_back(pfd);
	}

	int ret = poll(fds.data(), fds.size(), -1);
	if (ret < 0)
	{
		Logger::Log(LogLevel::ERROR, "Poll error: " + std::string(strerror(errno)));
		return;
	}

	for (size_t i = 0; i < fds.size(); ++i)
	{
		if (fds[i].revents & POLLIN)
		{
			if (i == 0) // Listening socket
			{
				// Handle new connection directly here
				struct sockaddr_in client_addr;
				socklen_t addrlen = sizeof(client_addr);
				int client_fd = accept(_listening_socket.socket.getSocketFd(), (struct sockaddr*)&client_addr, &addrlen);
				
				if (client_fd < 0)
					Logger::Log(LogLevel::ERROR, "Accept error: " + std::string(strerror(errno)));
				else
				{
					_listening_socket.socket.setNonBlockingSocket(client_fd);
					Socket clientSocket(_config);
					clientSocket.setSocketFd(client_fd);
					_sockets.emplace_back(client_fd, -1, e_socket_state::READ, clientSocket, std::stringstream());
				}
			}
			else // Client socket
			{
				_sockets[i].state = e_socket_state::READ;
				char buffer[_config.getClientMaxBodySize()];
				ssize_t bytes = recv(_sockets[i].fd, buffer, sizeof(buffer), 0);
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
				else
				{
					_sockets[i].buffer.write(buffer, bytes);
				}
			}
		}
	}
}

e_complete_data Server::isDataComplete(t_socket_data &socket) // used an enum
{
	std::string data = socket.buffer.str();
	//need to rework this

	// Case 1: Check if data is empty or malformed ->Incomplete
	if (data.empty() || data.find("\r\n\r\n") == std::string::npos)
		return e_complete_data::INCOMPLETE;
	
	// Case 2: Check for chunked transfer -> Chunked finished or unfinished
	if (data.find("Transfer-Encoding: chunked") != std::string::npos)
	{
		if (data.find("0\r\n\r\n") != std::string::npos)
			return e_complete_data::CHUNKED_FINISHED;
		return e_complete_data::CHUNKED_UNFINISHED;
	}
	
	// Case 3: Check Content-Length -> Complete or Incomplete (but doesnt make sense now need to rework)
	size_t contentLengthPos = data.find("Content-Length: ");
	if (contentLengthPos != std::string::npos)
	{
		size_t headerEnd = data.find("\r\n\r\n");
		std::string lengthStr = data.substr(contentLengthPos + 16, 
		data.find("\r\n", contentLengthPos) - (contentLengthPos + 16));
		
		try
		{
			size_t contentLength = std::stoul(lengthStr);
			size_t bodyLength = data.length() - (headerEnd + 4);
			
			if (bodyLength >= contentLength)
				return e_complete_data::COMPLETE;
			return e_complete_data::INCOMPLETE;
		}
		catch (...)
			return e_complete_data::INCOMPLETE;
		return e_complete_data::INCOMPLETE;
	}
}

void Server::Run()
{
	while (true)
	{
		// Monitor and read from sockets
		updatePoll();

		// Process data and handle states
		for (size_t i = 0; i < _sockets.size(); ++i) // Start from 1 to skip listening socket that recieves new data
		{
			t_socket_data &sockets = _sockets[i];
			
			switch (sockets.state)
			{
				case e_socket_state::READ:
					if (isDataComplete(sockets))
					{
						sockets.state = e_socket_state::WRITE;
					}
					else
					{
						// Handle incomplete data
					}
					break;

				case e_socket_state::WRITE:
					// Send response using sendData()
					// Example:
					// Response response = generateResponse(socket_data);
					// socket_data.socket.sendData(response, socket_data.fd);
					
					sockets.state = e_socket_state::READ;
					sockets.buffer.str(""); // Clear buffer after sending
					break;

				case e_socket_state::CLOSE:
					// Close and remove the socket
					_sockets[i].socket.closeSocket(_sockets[i].fd);
					_sockets.erase(_sockets.begin() + i);
					--i;
					break;
				case e_socket_state::UNKNOWN:
					break;
				case e_socket_state::ACCEPT:
					continue;
				case e_socket_state::CONNECT:
					continue;
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