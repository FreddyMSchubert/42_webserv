#include "../../../include/Packets/Request.hpp"

static void handle_file_req(t_server_config &config, Response &response, Request &req)
{
	std::string path = req.getPath();
	if (path == std::string("/"))
		response.setPath(config.default_location.index); // TODO: i think we should loop trough all the paths and pick the first one to exist
	else
		response.setPath(path);

	path = response.getPath();

	response.setVersion("HTTP/1.1");

	try
	{
		std::string file = getFileAsString(std::string(config.default_location.root) + path);
		response.addHeader("Content-Length", std::to_string(file.size()));
		response.setStatus(Status::OK);
		response.addHeader("Content-Type", "text/" + path.substr(path.find_last_of('.') + 1) + "; charset=UTF-8");
		response.setBody(file);
	}
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::ERROR, std::string("Failed to get file: ") + e.what());
		response = Response(); // TODO: do the actual error handling here
		response.setStatus(Status::NotFound);
	}
}

static void handle_dir_req(t_server_config &config, Response &response, Request &req)
{
	(void)config;
	(void)response;
	(void)req;

	// if path is /, we should return the index file if there is one and if not, we should return a list of files in the directory
	// and if the directory does not exist we return 404
}

static bool is_file_req(Request &req)
{
	std::string path = req.getPath();
	if (path[path.length() - 1] == '/') // XXX: dont know what they mean with /$ at the end of the path.
		return false;
	return true;
}

void Request::handleGet(t_server_config &config, Response &response)
{

	if (is_file_req(*this))
		handle_file_req(config, response, *this);
	else
		handle_file_req(config, response, *this);
	
}
