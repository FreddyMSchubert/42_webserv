#pragma once

#include <iostream>
#include "Packet.hpp"
#include "../../include/Logger.hpp"
#include "../../include/Socket.hpp"
#include "../../include/Utils.hpp"
#include "../../include/Enums.hpp"
#include "../../include/Config.hpp"

class Request : public Packet
{
	private:
		void handleGet(t_server_config config, Response &response);
		void handlePost(t_server_config config, Response &response);
		void handleDelete(t_server_config config, Response &response);
	public:
		Request(const std::string &rawPacket) : Packet(rawPacket) {};
		~Request() = default;
		Response ProcessRequest(t_server_config config);
};