#include "Server.hpp"
#include "Socket.hpp"

Server::Server(Config &config) : _config(config)
{
	Socket listeningSocket(config);
	listeningSocket.connectSocket();
	listeningSocket.setNonBlockingSocket(listeningSocket.getSocketFd());

	_sockets.push_back(listeningSocket);
	_socket_data.emplace_back(listeningSocket.getSocketFd(), -1, e_socket_state::ACCEPT, listeningSocket, std::stringstream());

	Logger::Log(LogLevel::INFO, "Server listening on " + _config.getHost() + ":" + std::to_string(_config.getPort()) + ".");
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
		pfd.fd = _sockets[i].getSocketFd();
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
				socklen_t addrlen = sizeof(_sockets[0]);
				int client_fd = accept(_sockets[0].getSocketFd(), (struct sockaddr*)&_sockets[0], &addrlen);
				
				if (client_fd < 0)
					Logger::Log(LogLevel::ERROR, "Accept error: " + std::string(strerror(errno)));
				else
				{
					_sockets[0].setNonBlockingSocket(client_fd);
					Socket clientSocket(_config);
					clientSocket.setSocketFd(client_fd);
					_sockets.push_back(clientSocket);
					_socket_data.emplace_back(client_fd, -1, e_socket_state::READ, clientSocket, std::stringstream());
				}
			}
			else // Client socket
			{
				_socket_data[i].state = e_socket_state::READ;
				char buffer[_config.getClientMaxBodySize()];
				ssize_t bytes = recv(_socket_data[i].fd, buffer, sizeof(buffer), 0);
				if (bytes <= 0 || bytes > _config.getClientMaxBodySize())
				{
					if (bytes <= 0)
						Logger::Log(LogLevel::INFO, "Connection closed by client");
					else if (bytes > _config.getClientMaxBodySize()) //as far as i know if that gets triggered we should display a 413 http error
						Logger::Log(LogLevel::ERROR, "Client sent too much data, closing socket");
					_socket_data[i].state = e_socket_state::CLOSE;
					_sockets[i].closeSocket(_socket_data[i].fd);
					_sockets.erase(_sockets.begin() + i);
					_socket_data.erase(_socket_data.begin() + i);
					--i;
				}
				else
				{
					_socket_data[i].buffer.write(buffer, bytes);
				}
			}
		}
	}
}

// IsClosed function in issue
bool Server::isDataComplete(t_socket_data &socket) // cant be a bool need true, false, and shit broke go fix it as third state -> no why? either what the client sends is correct or its not. why do i need to fix something that the client failed to do?
{
    std::string data = socket.buffer.str();

    // Find the end of headers0

    size_t header_end = data.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;

    std::string headers = data.substr(0, header_end);
    std::istringstream header_stream(headers);
    std::string line;
    size_t content_length = 0;
    bool chunked = false;

    // Parse headers
    while (std::getline(header_stream, line) && line != "\r") {
        // Convert header line to lowercase for case-insensitive comparison
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        if (line.find("content-length:") != std::string::npos) {
            content_length = std::stoul(line.substr(15)); // Extract value after "content-length: "
        }
        else if (line.find("transfer-encoding:") != std::string::npos) {
            if (line.find("chunked") != std::string::npos)
                chunked = true;
        }
    }

    // Determine message completeness based on encoding
    if (chunked) {
        // Handle chunked encoding
        // Check if the terminating chunk is received (0\r\n\r\n)
        if (data.find("0\r\n\r\n") != std::string::npos)
            return true;
        else
            return false;
    }
    else if (content_length > 0) {
        // Check if body is fully received
        size_t total_length = header_end + 4 + content_length;
        return data.size() >= total_length;
    }
    else {
        // No body or unrecognized encoding
        return true; // Consider headers complete
    }
}

void Server::Run()
{
	while (true)
	{
		// Monitor and read from sockets
		updatePoll();

		// Process data and handle states
		for (size_t i = 1; i < _socket_data.size(); ++i) // Start from 1 to skip listening socket that recieves new data
		{
			t_socket_data &socket_data = _socket_data[i];
			
			switch (socket_data.state)
			{
				case e_socket_state::READ:
					if (isDataComplete(socket_data))
					{
						socket_data.state = e_socket_state::WRITE;
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
					
					socket_data.state = e_socket_state::READ;
					socket_data.buffer.str(""); // Clear buffer after sending
					break;

				case e_socket_state::CLOSE:
					// Close and remove the socket
					_sockets[i].closeSocket(_socket_data[i].fd);
					_sockets.erase(_sockets.begin() + i);
					_socket_data.erase(_socket_data.begin() + i);
					--i;
					break;
			}
		}
	}
}