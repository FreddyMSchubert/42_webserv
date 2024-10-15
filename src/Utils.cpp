#include "Utils.hpp"

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

bool isAllowedMethodAt(t_server_config &config, const t_location location, Method method)
{
	t_location loc = location;

	do
	{

		if (std::find(loc.allowed_methods.begin(), loc.allowed_methods.end(), method) != loc.allowed_methods.end())
			return true;

		std::string parentPath = loc.root.substr(0, loc.root.find_last_of('/'));
		if (parentPath.empty() || loc.root == config.default_location.root)
			break;

		loc = get_location(config, parentPath);

	} while (true);

	return (std::find(config.default_location.allowed_methods.begin(), config.default_location.allowed_methods.end(), method) != config.default_location.allowed_methods.end());
}

std::vector<std::filesystem::directory_entry> getDirectoryEntries(const std::string& path)
{
	std::vector<std::filesystem::directory_entry> entries;
	for (const auto& entry : std::filesystem::directory_iterator(path))
		entries.push_back(entry);
	return entries;
}

bool isSubroute(const std::string& route, const std::string& subroute)
{
	return subroute.find(route) == 0;
}

std::string getFilePathAsURLPath(std::string path, t_server_config &config)
{
	std::string urlPath = path;
	urlPath.replace(0, config.default_location.root.size(), "");
	if (urlPath == "")
		urlPath = "/";
	return urlPath;
}

// Gets the location config of the path, respecting subdirs
t_location get_location(t_server_config &config, std::string path)
{
	t_location loc = (t_location){{}, "", "", false, 0, false};

	if (getFilePathAsURLPath(config.default_location.root, config) == path && std::filesystem::exists(config.default_location.root))
		loc = config.default_location;

	for (t_location &location : config.locations)
		if (isSubroute(getFilePathAsURLPath(location.root, config), path) && std::filesystem::exists(config.default_location.root))
			if (loc.root.size() < location.root.size())
				loc = location;

	return loc;
}