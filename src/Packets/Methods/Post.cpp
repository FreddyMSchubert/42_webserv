#include "Response.hpp"
#include <fcntl.h>
#include <unistd.h>
#include "Utils.hpp"
#include "./mimetypes.cpp"

void Response::handlePost(Request& req, Config &config)
{
	(void)config;
	
	// TODO: Check if on route POST is allowed
	// reutrn 405 (FORBIDDEN) if not allowed

	std::string content = req.getBody();
	std::string path = req.getPath();
	int content_length;

	setVersion("HTTP/1.1");

	t_location location = get_location(config, Path(path, Path::Type::URL, config).asFilePath());

	if (!get_location(config, Path(path, Path::Type::URL, config).asUrl()).allowed_methods[static_cast<int>(Method::POST)])
	{
		std::cerr << "POST method not allowed" << std::endl;
		setStatus(Status::Forbidden);
		setBody("POST method not allowed");
		return;
	}

	if (req.getHeaders().find("Content-Length") == req.getHeaders().end())
	{
		std::cerr << "No Content-Length header" << std::endl;
		setStatus(Status::Forbidden);
		setBody("No Content-Length header");
		return;
	}

	content_length = std::stoi(req.getHeaders()["Content-Length"]);

	if (req.getHeaders().find("Content-Type") == req.getHeaders().end())
	{
		std::cerr << "No Content-Type header" << std::endl;
		setStatus(Status::Forbidden);
		setBody("No Content-Type header");
		return ;
	}

	std::string filename;
	try
	{
		Path filepath = Path(req.getPath(), Path::Type::URL, config);
		if (req.getHeaders().find("X-Filename") != req.getHeaders().end())
			filename = filepath.asFilePath() + req.getHeaders()["X-Filename"];
		else
			filename = filepath.asFilePath() + "default";
	}
	catch (std::exception &e)
	{
		std::cerr << "Error parsing path" << std::endl;
		setStatus(Status::Forbidden);
		setBody("Error parsing path");
		return ;
	}

	std::string extension = "txt";
	try {
		for (const auto &mimeType : mimeTypes)
		{
			if (mimeType.second == req.getHeaders()["Content-Type"])
			{
				extension = mimeType.first;
				break ;
			}
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "Error parsing Content-Type" << std::endl;
		setStatus(Status::Forbidden);
		setBody("Error parsing Content-Type");
		return ;
	}
	filename += "." + extension;

	std::cout << "Filename: " << filename << std::endl;

	int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644); // TODO: check if the flags are correct

	if (fd == -1)
	{
		std::cerr << "Error opening file" << std::endl;
		setStatus(Status::Forbidden);
		setBody("Error opening file");
		return ;
	}

	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Failed to get socket flags");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set non-blocking mode");

	if (write(fd, content.c_str(), content_length) == -1)
	{
		close(fd);
		std::cerr << "Error writing to file" << std::endl;
		setStatus(Status::Forbidden);
		setBody("Error writing to file");
		return ;
	}

	setStatus(Status::OK);
	setBody("File uploaded successfully");
	close(fd);

	std::cout << "Handling POST request" << std::endl;
}
