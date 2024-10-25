#include "Utils.hpp"

bool isAllowedMethodAt(t_server_config &config, Path path, Method method)
{
	while (true)
	{
		std::cout << "Attempting to find method " << method << " at " << path << std::endl;

		t_location location = get_location(config, path.asFilePath());
		if (location.root == "/" || location.root.empty())
			break;
		auto methodPair = location.allowed_methods.find(method);
		if (methodPair != location.allowed_methods.end())
			return methodPair->second;

		path.goUpOneDir();
		if (path.isRoot())
			break;
	};

	auto foundLoc = config.default_location.allowed_methods.find(method);
	if (foundLoc != config.default_location.allowed_methods.end())
		return foundLoc->second;
	return false; // default for all methods if no rule is defined
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
	t_location loc = EMPTY_LOCATION;

	std::cout << "Getting location for path: \"" << path << "\"" << " at root " << config.default_location.root << std::endl;
	if (path == config.default_location.root)
	{
		std::cout << "The path is \"/\"." << std::endl;
		if (std::filesystem::exists(config.default_location.root))
		{
			loc = config.default_location;
			std::cout << "Default location is valid." << std::endl;
		}
	}

	std::cout << "Initial location: " << loc.root << std::endl;

	for (t_location &location : config.locations)
	{
		std::cout << "Current location: " << loc.root << " checking against " << location.root << std::endl;
		if (isSubroute(location.root, path) && std::filesystem::exists(location.root))
			if (loc.root.size() < location.root.size())
				loc = location;
	}

	return loc;
}
