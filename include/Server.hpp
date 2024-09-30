#pragma once

#include "Enums.hpp"

#include <iostream>

class Server
{
	private:
		int _port;
		bool _run;
		std::string _config_file;

	public:
		Server(const std::string &config_file, int port);
		~Server();
		
		void Run();
};