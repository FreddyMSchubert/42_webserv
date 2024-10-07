#include "../include/Tests.hpp"

void Tests::testPacketParsing(std::string testPacketPath)
{
	std::ifstream file(testPacketPath);
	if (!file.is_open() || !file.good())
		throw std::runtime_error("Couldn't open packet testing file");
	std::stringstream buffer;
	buffer << file.rdbuf();
	Packet packet = Packet(buffer.str());
	packet.logData();
}
