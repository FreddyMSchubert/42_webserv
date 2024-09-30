#pragma once

#include "Enums.hpp"

#include <iostream>

class Webserv
{
	private:
		int _port;
		bool _run;
		std::string _config_file;

	public:
		Webserv(const std::string &config_file, int port);
		~Webserv();
		
		void run();
};