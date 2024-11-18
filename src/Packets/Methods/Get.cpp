#include "Response.hpp"

void Response::handle_file_req(Config &config, FilePath &path)
{
	setPath(path.asUrl());

	if (!isAllowedMethodAt(config, path, Method::GET))
	{
		Logger::Log(LogLevel::WARNING, "GET: Method not allowed");
		setStatus(Status::MethodNotAllowed);
		return;
	}

	std::cout << "Method allowed" << std::endl;

	try
	{
		std::string file = path.getFileContents();
		addHeader("Content-Length", std::to_string(file.size()));
		addHeader("Content-Type", path.getMimeType() + "; charset=UTF-8");
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
			body += "<li>📄 <a target=\"_blank\" href=\"" + entry.path().filename().string() + "\">" + entry.path().filename().string() + "</a></li>";
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

void Response::handle_dir_req(Config &config, Path &path)
{
	t_location location = get_location(config, path.asFilePath());

	if (location.empty()) // invalid
	{
		Logger::Log(LogLevel::WARNING, "GET: Invalid location \"" + path.asUrl() + "\"");
		setStatus(Status::NotFound);
		return;
	}

	if (!isAllowedMethodAt(config, path, Method::GET))
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

	std::vector<std::filesystem::directory_entry> entries = path.getDirectoryEntries();

	std::string body = get_dir_list_html(path.asUrl(), entries);

	addHeader("Content-Length", std::to_string(body.size()));
	setStatus(Status::OK);
	addHeader("Content-Type", "text/html; charset=UTF-8");

	setBody(body);
}

void Response::handleGet(Request& req, Config &config)
{
	Logger::Log(LogLevel::INFO, "GET: Handling request");

	std::string reqTarget = req.getPath();
	if (reqTarget == "/")
		reqTarget = config.getIndexFile().asUrl();
	std::variant<Path, FilePath> path = createPath(reqTarget, Path::Type::URL, config);

	setVersion("HTTP/1.1");

	if (std::holds_alternative<FilePath>(path))
	{
		Logger::Log(LogLevel::INFO, "GET: Handling file request for " + std::get<FilePath>(path).asUrl());
		handle_file_req(config, std::get<FilePath>(path));
	}
	else
	{
		Logger::Log(LogLevel::INFO, "GET: Handling directory request for " + std::get<Path>(path).asUrl());
		handle_dir_req(config, std::get<Path>(path));
	}
}
