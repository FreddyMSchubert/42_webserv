#pragma once

#include "Packet.hpp"
#include <iostream>

class Response : public Packet
{
	public:
		Response() : Packet() {};
		Response(const std::string &rawPacket) : Packet(rawPacket) {};
		~Response() = default;
		void Run();
		std::string getRawPacket();
};