#include "Response.hpp"

void Response::handleDelete(Request& req, Config &config)
{
	(void)config;
	(void)req;
	std::cout << "DELETE request received" << std::endl;
}