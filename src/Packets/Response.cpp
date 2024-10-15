#include "Response.hpp"

Response::Response(Request& req, t_server_config &config) : Packet()
{
	switch (req.getMethod())
	{
		case Method::GET:
			handleGet(req, config);
			break;
		case Method::POST:
			handlePost(req, config);
			break;
		case Method::DELETE:
			handleDelete(req, config);
			break;
		default:
			std::cout << "Unknown method" << std::endl;
			break;
	}
}

/* ----- METHODS ----- */

std::string Response::getRawPacket()
{
	std::string rawData;

	rawData +=  getVersion() + " " + std::to_string((int)getStatus()) + "\r\n";

	for (auto &header : getHeaders())
		rawData += header.first + ": " + header.second + "\r\n";
	
	rawData += "\r\n" + getBody();
	rawData += "\r\n\r\n";

	return rawData;
}
