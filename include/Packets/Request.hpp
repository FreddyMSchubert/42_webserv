#pragma once

#include <iostream>
#include "Packet.hpp"

class Request : public Packet
{

	public:
		Request(const std::string &rawPacket) : Packet(rawPacket) {};
		~Request() = default;
		void Run();

};