#include "Response.hpp"
#include <dirent.h>

void Response::handleDelete(Request& req, Config &config)
{
	std::string content = req.getBody();
	std::string path = req.getPath();

	setVersion("HTTP/1.1");

	t_location location = get_location(config, Path(path, Path::Type::URL, config).asFilePath());

	if (!get_location(config, Path(path, Path::Type::URL, config).asUrl()).allowed_methods[static_cast<int>(Method::DELETE)])
	{
		std::cerr << "DELETE method not allowed" << std::endl;
		setStatus(Status::Forbidden);
		setBody("DELETE method not allowed");
		return;
	}

	// FIXME: sometimes the path is corrent and sometimes not
	path = "." + location.root_dir + path;
	std::cout << "Path: " << path << std::endl;

	// delete file
	if(path[path.length() - 1] != '/')
	{
		std::cout << "Trying to delete file: " << path << std::endl;
		if (remove(path.c_str()) != 0)
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
		return;
	}
	
	// delete directory
	DIR *dir;
	struct dirent *ent;

	std::cout << "Trying to delete directory: " << path << std::endl;

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			std::string file = ent->d_name;
			if (file != "." && file != "..")
			{
				std::string filepath = path + file;
				if (remove(filepath.c_str()) != 0)
				{
					std::cerr << "Error deleting file" << std::endl;
					setStatus(Status::Forbidden);
					setBody("Error deleting file");
					return;
				}
			}
		}
		closedir(dir);
		if (rmdir(path.c_str()) != 0)
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
