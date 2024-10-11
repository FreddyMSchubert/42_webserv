#include "../../include/Packets/Request.hpp"

// TODO: this function should not return void but the response to the request that we can send it back directly
std::string Request::ProcessRequest()
{
	std::cout << "Request::Run()" << std::endl;

	std::string path = getPath();
	if (path == std::string("/"))
		path = "/index.html";

	std::string response_str;
	try
	{
		return getFileAsString(std::string("./www") + path);
	}
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::ERROR, std::string("Failed to get file: ") + e.what());
		return std::string();
	}
}
