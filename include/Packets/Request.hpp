#pragma once

#include <iostream>
#include "Packet.hpp"
#include "../../include/Logger.hpp"
#include "../../include/Socket.hpp"
#include "../../include/Utils.hpp"
#include "../../include/Enums.hpp"

class Request : public Packet
{
	public:
		Request(const std::string &rawPacket) : Packet(rawPacket) {};
		~Request() = default;
		std::string ProcessRequest();
};