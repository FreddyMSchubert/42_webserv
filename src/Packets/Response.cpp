#include "../../include/Packets/Response.hpp"

void Response::Run()
{
	std::cout << "Response::Run()" << std::endl;
}

std::string Response::getRawPacket()
{
	std::string rawData;

	rawData +=  _version + " " + std::to_string((int)_status) + "\r\n";

	for (auto &header : _headers)
		rawData += header.first + ": " + header.second + "\r\n";
	
	rawData += "\r\n" + _body;
	rawData += "\r\n\r\n";

	return rawData;
}