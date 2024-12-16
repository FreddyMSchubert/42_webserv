#include "Response.hpp"

void Response::handleCgiResponse(Request &req, Config &config)
{
	FilePath fpath(req.getPath(), Path::Type::URL, config);
	std::string extension = fpath.getFileExtension();
	std::string exec_cmd;

	std::vector<t_location> locations = get_locations(config, req.getPath());
	for (t_location loc : locations)
	{
		std::vector<std::pair<std::string, std::string>> cgi_extensions = loc.cgi_extensions;
		for (std::pair<std::string, std::string> ext : cgi_extensions)
		{
			if (ext.first == extension)
			{
				exec_cmd = ext.second;
				break;
			}
		}
	}
}
