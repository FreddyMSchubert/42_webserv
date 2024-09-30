#include "../include/Server.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	
	try
	{
		Server webserv(argc == 2 ? argv[1] : "./config/default.conf", 80);
		webserv.Run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	

}