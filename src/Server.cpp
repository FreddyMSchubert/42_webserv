#include "Server.hpp"
#include "Socket.hpp"

//Setup the listening socket and push it to the vector of sockets
Server::Server(Config &config) : _config(config)
{
	Socket listeningSocket(config);
	listeningSocket.connectSocket();
	listeningSocket.setNonBlockingSocket(listeningSocket.getSocketFd());

	_socket_data.emplace_back(listeningSocket.getSocketFd(), -1, e_socket_state::ACCEPT, listeningSocket, std::stringstream());

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
	for (size_t i = 0; i < _socket_data.size(); ++i)
	{
		struct pollfd pfd;
		pfd.fd = _socket_data[i].socket.getSocketFd();
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
				socklen_t addrlen = sizeof(_socket_data[0].socket); // what in the world is this // why would you pass the size of our class obj to accept?
				int client_fd = accept(_socket_data[0].socket.getSocketFd(), (struct sockaddr*)&_socket_data[0].socket, &addrlen);
				
				if (client_fd < 0)
					Logger::Log(LogLevel::ERROR, "Accept error: " + std::string(strerror(errno)));
				else
				{
					_socket_data[0].socket.setNonBlockingSocket(client_fd);
					Socket clientSocket(_config);
					clientSocket.setSocketFd(client_fd);
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
					_socket_data[i].socket.closeSocket(_socket_data[i].fd);
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


bool Server::isDataComplete(t_socket_data &socket) // We still got the problem that we need 3 states: 1) chunked 2) complete message recieved 3) incomplete/false message
{
    std::string data = socket.buffer.str();

    size_t header_end = data.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;

    std::string headers = data.substr(0, header_end);
    std::istringstream header_stream(headers);
    std::string line;
    size_t content_length = 0;
    bool chunked = false;

    // Parse headers
    while (std::getline(header_stream, line) && line != "\r")
	{
        // Convert header line to lowercase for case-insensitive comparison
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        if (line.find("content-length:") != std::string::npos)
            content_length = std::stoul(line.substr(15)); // Extract value after "content-length: "
        else if (line.find("transfer-encoding:") != std::string::npos)
		{
            if (line.find("chunked") != std::string::npos)
                chunked = true;
        }
    }

    // Determine message completeness based on encoding
    if (chunked)
	{
        if (socket.buffer.str().size() > _config.getClientMaxBodySize())
			return false;
        if (data.find("0\r\n\r\n") != std::string::npos)
		{
			// Process the chunked data (remove the chunked encoding and combine the chunks)
            return true;
		}
        else
            return false;
    }
    else if (content_length > 0)
	{
        size_t total_length = header_end + 4 + content_length;
        return data.size() >= total_length;
    }
	return false;
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
					_socket_data[i].socket.closeSocket(_socket_data[i].fd);
					_socket_data.erase(_socket_data.begin() + i);
					--i;
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