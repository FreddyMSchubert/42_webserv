#include "../../../include/Packets/Request.hpp"

void Request::handleDelete(t_server_config config, Response &response)
{
	(void)config;
	(void)response;
	std::cout << "DELETE request received" << std::endl;
}