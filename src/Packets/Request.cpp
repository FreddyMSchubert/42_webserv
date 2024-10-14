#include "../../include/Packets/Request.hpp"

Response Request::ProcessRequest(t_server_config config)
{
	Response response;

	switch (_method)
	{
		case Method::GET:
			handleGet(config, response);
			break;
		case Method::POST:
			handlePost(config, response);
			break;
		case Method::DELETE:
			handleDelete(config, response);
			break;
		default:
			std::cout << "Unknown method" << std::endl;
			break;
	}

	return response;

	// std::string path = getPath();
	// if (path == std::string("/"))
	// 	path = config.default_location.index;

	// std::string response_str;
	// response_str += "HTTP/1.1 200 OK\r\n";
	// response_str += "Content-Type: text/" + path.substr(path.find_last_of('.') + 1) + "; charset=UTF-8\r\n";
	// try
	// {
	// 	std::string file = getFileAsString(std::string(config.default_location.root) + path);
	// 	response_str += "Content-Length: " + std::to_string(file.size()) + "\r\n";
	// 	response_str += "\r\n";
	// 	response_str += file;
	// 	return response_str;
	// }
	// catch (std::exception &e)
	// {
	// 	Logger::Log(LogLevel::ERROR, std::string("Failed to get file: ") + e.what());
	// 	return std::string();
	// }
}
