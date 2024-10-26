#pragma once

#include <iostream>
#include "Packet.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include "Enums.hpp"
#include "Config.hpp"

class Request : public Packet
{
	private:
		// parsing
		void ParseRequestLine(std::string &line);
		void ParseHeaders(std::string &headers);
		void ParseBody(std::string &body);
		std::string sanitizeUri(const std::string& uri);
	public:
		Request(const std::string &rawPacket);
		~Request() = default;
};