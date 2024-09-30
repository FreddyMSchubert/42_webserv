#include "../include/Webserv.hpp"
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
		Webserv webserv(argc == 2 ? argv[1] : "./config/default.conf", 80);
		webserv.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	

}