#include "Response.hpp"

Response::Response(Request& req, Config &config) : Packet()
{
	bool cgi = false;
	std::vector<t_location> locations = get_locations(config, req.getPath());
	std::string fileEnding = req.getPath().substr(req.getPath().find_last_of('.') + 1);
	if (fileEnding.size() > 0 && locations.size() > 0 && (req.getMethod() == Method::GET || req.getMethod() == Method::POST))
	{
		for (t_location &loc : locations)
		{
			std::vector<std::pair<std::string, std::string>> cgi_extensions = loc.cgi_extensions;
			for (std::pair<std::string, std::string> &ext : cgi_extensions)
			{
				if (ext.first == fileEnding)
				{
					cgi = true;
					break;
				}
			}
			if (cgi)
				break;
		}
	}

	if (cgi)
		handleCgiResponse(req, config);
	else
		handleMethodResponse(req, config);
}

void Response::handleMethodResponse(Request &req, Config &config)
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
			setStatus(Status::MethodNotAllowed);
			break;
	}
}
