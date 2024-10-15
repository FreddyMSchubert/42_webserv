#include "Response.hpp"

void Response::handleDelete(Request& req, t_server_config &config)
{
	(void)config;
	(void)req;
	std::cout << "DELETE request received" << std::endl;
}