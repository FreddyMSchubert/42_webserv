#include "../../../include/Packets/Request.hpp"

void Request::handlePost(t_server_config config, Response &response)
{
	(void)config;
	(void)response;
	std::cout << "Handling POST request" << std::endl;
}