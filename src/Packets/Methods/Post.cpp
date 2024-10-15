#include "Response.hpp"

void Response::handlePost(Request& req, t_server_config &config)
{
	(void)config;
	(void)req;
	std::cout << "Handling POST request" << std::endl;
}