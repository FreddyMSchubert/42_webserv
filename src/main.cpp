#include "../include/Server.hpp"
#include "../include/Logger.hpp"
#include <iostream>
#include "../include/Tests.hpp"

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}

	// Tests::testPacketParsing("./tester/packet_parsing_tests/basic_get.packet");
	// Tests::testPacketParsing("./tester/packet_parsing_tests/basic_post.packet");
	// Tests::testPacketParsing("./tester/packet_parsing_tests/basic_delete.packet");
	
	try
	{
		Server webserv(argc == 2 ? argv[1] : "./config/default.conf", 80);
		webserv.Run();
	}
	catch(const std::exception& e)
	{
		Logger::Log(LogLevel::ERROR, e.what());
	}
	
	return 0;
}