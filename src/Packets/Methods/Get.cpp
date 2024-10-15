#include "Response.hpp"

void Response::handle_file_req(t_server_config &config, Request &req)
{
	std::string path = req.getPath();
	if (path == "/")
		setPath(config.default_location.index); // TODO: consider looping through all possible index files
	else
		setPath(path);

	path = getPath();

	std::cout << "Path: " << path << std::endl;

	if (isAllowedMethodAt(config, get_location(config, path), Method::GET) == false)
	{
		Logger::Log(LogLevel::WARNING, "GET: Method not allowed");
		setStatus(Status::MethodNotAllowed);
		return;
	}

	try
	{
		std::string file = getFileAsString(std::string(config.default_location.root) + path);
		addHeader("Content-Length", std::to_string(file.size()));
		std::string fileName = path.substr(path.find_last_of('/') + 1);
		addHeader("Content-Type", getMimeType(fileName) + "; charset=UTF-8");
		setStatus(Status::OK);
		setBody(file);
	}
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::ERROR, std::string("Failed to get file: ") + e.what());
		setBody("Failed to get file");
		setStatus(Status::NotFound);
	}
}

static std::string get_dir_list_html(const std::string &current_path, const std::vector<std::filesystem::directory_entry> &entries)
{
	std::string body = "<html><head><title>Directory Listing</title></head><body><div class=\"floating\"><h1>Directory Listing</h1><ul>";

	body += "<li>🏠 <a href=\"/\">Root</a></li>";

	if (current_path != "/")
	{
		std::string up_path = current_path;
		if (up_path.back() == '/')
			up_path.pop_back();
		size_t pos = up_path.find_last_of('/');
		if (pos != std::string::npos)
			up_path = up_path.substr(0, pos + 1);
		else
			up_path = "/";
		body += "<li>⬆️ <a href=\"" + up_path + "\">..</a></li>";
	}

	body += "<br>";

	for (std::filesystem::directory_entry entry : entries)
	{
		if (entry.is_directory())
			body += "<li>📂 <a href=\"" + entry.path().filename().string() + "/\">" + entry.path().filename().string() + "/</a></li>";
		else
			body += "<li>📄 <a href=\"" + entry.path().filename().string() + "\">" + entry.path().filename().string() + "</a></li>";
	}

	body += R"(</ul></div><style>
	body {
		font-family: sans-serif;
		color: black;

		background-image: url(https://wallpapercave.com/wp/wp10973816.jpg);
		background-size: cover;
	}
	a {
		color: black;
	}
	.floating {
		padding: 50px 30px 50px 30px;
		box-shadow: 0px 0px 39px 22px rebeccapurple;
		display: flex;
		flex-direction: column;
		align-items: center;
		align-self: center;
		position: absolute;
		top: 50%;
		left: 50%;
		transform: translate(-50%, -50%);
		background-color: #FFFFFF54;
		backdrop-filter: blur(10px);
		border-radius: 50px;
		min-width: 100px;
	}
	</style>)";
	
	body += "</body></html>";

	return body;
}

void Response::handle_dir_req(t_server_config &config, Request &req)
{
	t_location location = get_location(config, req.getPath());

	if (location.empty()) // invalid
	{
		Logger::Log(LogLevel::WARNING, "GET: Invalid location");
		setStatus(Status::NotFound);
		return;
	}

	if (!isAllowedMethodAt(config, location, Method::GET))
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

	std::cout << "Root: " << location.root << std::endl;

	std::vector<std::filesystem::directory_entry> entries = getDirectoryEntries(std::string(config.default_location.root) + req.getPath());

	std::string body = get_dir_list_html(req.getPath(), entries);

	addHeader("Content-Length", std::to_string(body.size()));
	setStatus(Status::OK);
	addHeader("Content-Type", "text/html; charset=UTF-8");

	setBody(body);
}

static bool is_file_req(Request &req, t_server_config &config)
{
	if (req.getPath().back() != '/')
		return true;

	std::cout << req.getPath() << "is the path" << std::endl;
	t_location loc = get_location(config, req.getPath());
	std::cout << loc << std::endl;
	if (loc.empty())
		return false;

	std::string path = std::string(loc.root + loc.index);
	std::cout << "Path: " << path  << " and " << req.getPath() << std::endl;

	std::cout << "Checking if file or directory exists: " << path << std::endl;
	return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

void Response::handleGet(Request& req, t_server_config &config)
{
	std::cout << "Handling GET request" << std::endl;

	std::cout << is_file_req(req, config) << std::endl;

	setVersion("HTTP/1.1");

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
