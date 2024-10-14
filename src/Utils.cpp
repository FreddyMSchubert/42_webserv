#include "../include/Utils.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

std::string getFileAsString(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open() || !file.good())
		throw std::runtime_error(std::string("Couldn't read file at ") + path);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

bool isFileInDirectory(const std::string& path, const std::string& file)
{
	try
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_regular_file() && entry.path().filename() == file)
				return true;
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		// Handle the error (e.g., log it)
		std::cerr << "Filesystem error: " << e.what() << std::endl;
	}
	return false;
}

bool isAllowedMethodAt(t_location &location, Method method)
{
	return (std::find(location.allowed_methods.begin(), location.allowed_methods.end(), method)
		!= location.allowed_methods.end());
}

std::vector<std::filesystem::directory_entry> getDirectoryEntries(const std::string& path)
{
	std::vector<std::filesystem::directory_entry> entries;
	for (const auto& entry : std::filesystem::directory_iterator(path))
		entries.push_back(entry);
	return entries;
}

// TODO: This function is not correct (we return empty location when going into a sub sub route)
t_location get_location(t_server_config &config, std::string path)
{
	if (path[path.length() - 1] == '/')
		path = path.substr(0, path.length() - 1);

	if (config.default_location.root.length() >= path.length() && config.default_location.root.substr(config.default_location.root.length() - path.length(), config.default_location.root.length()) == path)
		return config.default_location;

	for (t_location &location : config.locations)
	{
		if (location.root.length() >= path.length() && location.root.substr(location.root.length() - path.length(), location.root.length()) == path)
			return location;
	}
	return (t_location){{}, "", "", false, 0, false};
}