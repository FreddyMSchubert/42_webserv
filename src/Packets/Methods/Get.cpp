#include "Response.hpp"

void Response::handle_file_req(t_server_config &config, Request &req)
{
	std::string path = req.getPath();
	if (path == std::string("/"))
		setPath(config.default_location.index); // TODO: i think we should loop trough all the paths and pick the first one to exist
	else
		setPath(path);

	path = getPath();

	setVersion("HTTP/1.1");

	std::cout << "Path: " << path << std::endl;

	try
	{
		std::string file = getFileAsString(std::string(config.default_location.root) + path);
		addHeader("Content-Length", std::to_string(file.size()));
		setStatus(Status::OK);
		addHeader("Content-Type", "text/" + path.substr(path.find_last_of('.') + 1) + "; charset=UTF-8");
		setBody(file);
	}
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::ERROR, std::string("Failed to get file: ") + e.what());
		setBody("Failed to get file");
		setStatus(Status::NotFound);
	}
}

static std::string get_dir_list_html(std::vector<std::filesystem::directory_entry> &entries)
{
	std::string body = "<html><head><title>Directory Listing</title></head><body><h1>Directory listing</h1><ul>";

	for (std::filesystem::directory_entry entry : entries)
	{
		if (entry.is_directory())
			body += "<li><a href=\"" + entry.path().filename().string() + "/\">" + entry.path().filename().string() + "/</a></li>";
		else
			body += "<li><a href=\"" + entry.path().filename().string() + "\">" + entry.path().filename().string() + "</a></li>";
	}

	body += R"(</ul><style>
	body {
		font-family: sans-serif;
		background-image: url(https://www.dieter-schwarz-stiftung.de/assets/images/b/Banner_42-Heilbronn_Innovationsfabrik_%28c%29-Bernhard-Lattner-995b796b.jpg);
	};
	ul {
		background-color: lightgray;
	}
	</style>)";
	
	body += "</body></html>";

	return body;
}

void Response::handle_dir_req(t_server_config &config, Request &req)
{
	t_location location = config.default_location;

	if (req.getPath().size() > 1) // sub route
		location = get_location(config, req.getPath());

	if (location.empty()) // invalid
	{
		Logger::Log(LogLevel::WARNING, "GET: Invalid location");
		setStatus(Status::NotFound);
		return;
	}

	if (isSubroute(location.root, req.getPath()))
	{
		if (!isAllowedMethodAt(location, Method::GET))
		{
			Logger::Log(LogLevel::WARNING, "GET: Method not allowed");
			setStatus(Status::MethodNotAllowed);
			return;
		}
		if (location.directory_listing == false) // no directory listing allowed
		{
			Logger::Log(LogLevel::WARNING, "GET: Directory listing not allowed");
			setStatus(Status::Forbidden);
			return;
		}
	}
	else
	{
		Logger::Log(LogLevel::WARNING, "GET: Not valid subroute");
		response.setStatus(Status::NotFound);
		return ;
	}


	std::cout << "Root: " << location.root << std::endl;

	std::vector<std::filesystem::directory_entry> entries = getDirectoryEntries(std::string(location.root));

	std::string body = get_dir_list_html(entries);

	addHeader("Content-Length", std::to_string(body.size()));
	setStatus(Status::OK);
	setVersion("HTTP/1.1");
	addHeader("Content-Type", "text/html; charset=UTF-8");

	setBody(body);

	// if path is /, we should return the index file if there is one and if not, we should return a list of files in the directory
	// and if the directory does not exist we return 404
}

static bool is_file_req(Request &req, t_server_config &config)
{
	if (req.getPath().back() != '/')
		return true;

	t_location loc = get_location(config, req.getPath());
	if (loc.empty())
		return false;

	std::string path = std::string(loc.root);
	std::cout << "Path: " << path  << " and " << req.getPath() << std::endl;

	if (req.getPath().size() > 1)
		path += req.getPath();

	if (path.back() == '/')
	{
		std::cout << "Checking if directory exists: " << path + loc.index << std::endl;
		return std::filesystem::exists(path + "index.html") && std::filesystem::is_regular_file(path + "index.html");
	}

	return true;
}

void Response::handleGet(Request& req, t_server_config &config)
{

	std::cout << "Handling GET request" << std::endl;

	if (is_file_req(req, config))
	{
		std::cout << "Handling file request" << std::endl;
		handle_file_req(config, req);
	}
	else
	{
		std::cout << "Handling directory request" << std::endl;
		handle_dir_req(config, req);
	}
	
}
