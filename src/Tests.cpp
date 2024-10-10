#include "../include/Tests.hpp"
#include "../include/Utils.hpp"

void Tests::testPacketParsing(std::string testPacketPath)
{
	Packet packet = Packet(getFileAsString(testPacketPath));
	packet.logData();
}
