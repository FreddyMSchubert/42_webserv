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

	std::cout << "Get is allowed for path " << path.asUrl() << std::endl;

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

static std::string get_dir_list_html(Path &current_path, const std::vector<std::filesystem::directory_entry> &entries)
{
	std::string body = getFileData("templates/directory_listing.html");

	// generate data to insert into the template
	std::ostringstream dataToInsert;

	if (!current_path.isRoot())
	{
		Path parentPath(current_path);
		parentPath.goUpOneDir();

		dataToInsert << "<li>⬆️ <a href=\"/" << parentPath.asUrl() << "\">..</a></li>";
	}

	dataToInsert << "<br>";

	for (std::filesystem::directory_entry entry : entries)
	{
		if (entry.is_directory())
			dataToInsert << "<li>📂 <a href=\"" + entry.path().filename().string() + "/\">" + entry.path().filename().string() + "/</a></li>";
		else
			dataToInsert << "<li>📄 <a target=\"_blank\" href=\"" + entry.path().filename().string() + "\">" + entry.path().filename().string() + "</a></li>";
	}

	const std::string needle = "[[[DIRECTORY_LISTING]]]";
	size_t pos = body.find(needle);
	if (pos != std::string::npos)
		body.replace(pos, needle.size(), dataToInsert.str());

	return body;
}

void Response::handle_dir_req(Config &config, Path &path)
{
	t_location location = get_location(config, path.asFilePath());

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

	std::string body = get_dir_list_html(path, entries);

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

	try
	{
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
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::ERROR, "Error getting data: " + std::string(e.what()));
		setStatus(Status::NotFound);
		return;
	}
}
