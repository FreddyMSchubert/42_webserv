#include "Response.hpp"
#include <dirent.h>
#include <filesystem>
#include <variant>

void Response::handleDelete(Request& req, Config &config)
{
	std::variant<Path, FilePath> optPath = createPath(req.getPath(), Path::Type::URL, config);
	

	std::string content = req.getBody();
	std::string path = req.getPath();

	setVersion("HTTP/1.1");

	t_location location;
	if (std::holds_alternative<Path>(optPath))
		location = get_location(config, std::get<Path>(optPath).asFilePath());
	else if (std::holds_alternative<FilePath>(optPath))
		location = get_location(config, static_cast<Path>(std::get<FilePath>(optPath)).asFilePath());
	else
		throw "That aint a path or a filepath you doofos";

	if (!location.allowed_methods[static_cast<int>(Method::DELETE)])
	{
		std::cerr << "DELETE method not allowed" << std::endl;
		setStatus(Status::Forbidden);
		setBody("DELETE method not allowed");
		return;
	}

	if (std::holds_alternative<FilePath>(optPath))
	{
		std::cout << "Trying to delete file: " << std::get<FilePath>(optPath).asFilePath() << std::endl;
		if (remove(std::get<FilePath>(optPath).asFilePath().c_str()) != 0)
		{
			std::cerr << "Error deleting file" << std::endl;
			setStatus(Status::Forbidden);
			setBody("Error deleting file");
		}
		else
		{
			setStatus(Status::OK);
			setBody("File deleted");
		}
	}
	else
	{
		DIR *dir;
		struct dirent *ent;

		std::cout << "Trying to delete directory: " << std::get<Path>(optPath).asFilePath() << std::endl;

		if ((dir = opendir(std::get<Path>(optPath).asFilePath().c_str())) != NULL)
		{
			while ((ent = readdir(dir)) != NULL)
			{
				std::string file = ent->d_name;
				if (file != "." && file != "..")
				{
					std::string filepath = path + file;
					if (remove(std::get<Path>(optPath).asFilePath().c_str()) != 0)
					{
						std::cerr << "Error deleting file" << std::endl;
						setStatus(Status::Forbidden);
						setBody("Error deleting file");
						return;
					}
				}
			}
			closedir(dir);
			if (rmdir(std::get<Path>(optPath).asFilePath().c_str()) != 0)
			{
				std::cerr << "Error deleting directory" << std::endl;
				setStatus(Status::Forbidden);
				setBody("Error deleting directory");
			}
			else
			{
				setStatus(Status::OK);
				setBody("Directory deleted");
			}
		}
		else
		{
			std::cerr << "Error opening directory" << std::endl;
			setStatus(Status::Forbidden);
			setBody("Error opening directory");
		}
	}

}
