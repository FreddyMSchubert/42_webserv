#pragma once

#include <fstream>
#include <sstream>
#include <exception>
#include "./Packets/Packet.hpp"

class Tests
{
	private:
		Tests() = delete;
		Tests(Tests const &src) = delete;
		Tests &operator=(Tests const &src) = delete;
	public:
		static void testPacketParsing(std::string testPacketPath);
};
