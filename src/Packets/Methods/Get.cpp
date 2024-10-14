#include "../../../include/Packets/Request.hpp"

void Request::handleGet(t_server_config config, Response &response)
{
	std::string path = getPath();
	if (path == std::string("/"))
		response.setPath(config.default_location.index);

	response.setVersion("HTTP/1.1");
	// response.addHeader("Content-Type", "text/" + path.substr(path.find_last_of('.') + 1) + "; charset=UTF-8");
	response.setMethod(Method::GET);

	try
	{
		std::string file = getFileAsString(std::string(config.default_location.root) + path);
		response.addHeader("Content-Length", std::to_string(file.size()));
		response.setBody(file);
	}
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::ERROR, std::string("Failed to get file: ") + e.what());
		response = Response(); // TODO: do the actual error handling here
	}

}
