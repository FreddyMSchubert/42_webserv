#include "Response.hpp"
#include <fcntl.h>
#include <unistd.h>
#include "Utils.hpp"

void Response::handlePost(Request& req, t_server_config &config)
{
	(void)config;
	
	// TODO: Check if on route POST is allowed
	// reutrn 405 (FORBIDDEN) if not allowed

	std::string content = req.getBody();
	std::string path = req.getPath();
	int content_length;

	setVersion("HTTP/1.1");

	if (req.getHeaders().find("Content-Length") == req.getHeaders().end())
	{
		std::cerr << "No Content-Length header" << std::endl;
		setStatus(Status::Forbidden);
		setBody("No Content-Length header");
		return ;
	}

	content_length = std::stoi(req.getHeaders()["Content-Length"]);

	if (req.getHeaders().find("Content-Type") == req.getHeaders().end())
	{
		std::cerr << "No Content-Type header" << std::endl;
		setStatus(Status::Forbidden);
		setBody("No Content-Type header");
		return ;
	}

	Path filepath = Path(req.getPath(), Path::Type::URL, config);

	std::string filename = filepath.asFilePath() + "test.txt"; // TODO: get real filename and filetype

	std::cout << "filename: " << filename << std::endl;

	int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644); // TODO: check if the flags are correct

	if (fd == -1)
	{
		std::cerr << "Error opening file" << std::endl;
		setStatus(Status::Forbidden);
		setBody("Error opening file");
		return ;
	}

	setNonBlocking(fd);

	if (write(fd, content.c_str(), content_length) == -1)
	{
		close(fd);
		std::cerr << "Error writing to file" << std::endl;
		setStatus(Status::Forbidden);
		setBody("Error writing to file");
		return ;
	}

	setStatus(Status::OK);
	close(fd);

	std::cout << "Handling POST request" << std::endl;
}