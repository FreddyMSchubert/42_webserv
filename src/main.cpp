#include "../include/Server.hpp"
#include <iostream>
#include "../include/Tests.hpp"

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}

	Tests::testPacketParsing("./tester/packet_parsing_tests/basic_get.packet");
	Tests::testPacketParsing("./tester/packet_parsing_tests/basic_post.packet");
	Tests::testPacketParsing("./tester/packet_parsing_tests/basic_delete.packet");
	Tests::testPacketParsing("./tester/packet_parsing_tests/chrome_get.packet");
	Tests::testPacketParsing("./tester/packet_parsing_tests/basic_chunked.packet");
	
	try
	{
		Server webserv(argc == 2 ? argv[1] : "./config/default.conf", 80);
		webserv.Run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}