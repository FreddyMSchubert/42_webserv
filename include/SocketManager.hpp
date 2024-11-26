#pragma once

#include <string>

#include "Config.hpp"
#include "Socket.hpp"

enum class e_socket_state
{
	READ,
	WRITE,
	ACCEPT,
	CONNECT,
	CLOSE
	// add more here i dont know what states actually exist
};

typedef struct s_socket_data
{
	int					fd;
	int					port;
	e_socket_state		state;
	Socket				&socket;
	std::stringstream	buffer;
}	t_socket_data;

class SocketManager
{
	private:
		Config &_config;
		std::vector<t_socket_data> _sockets;

		void updatePoll();
		bool isDataComplete(t_socket_data &socket);

	public:
		SocketManager(Config &config) : _config(config) {}
		SocketManager(SocketManager const &src) = delete;
		SocketManager &operator=(SocketManager const &src) = delete;
		SocketManager(SocketManager&& other) noexcept = default;
		~SocketManager() = default;
		
		void Run(); // loop through each conn, keep reading data, if error stop, if data is ready, construct response
};
