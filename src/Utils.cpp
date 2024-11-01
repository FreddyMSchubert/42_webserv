#include "Utils.hpp"

bool isAllowedMethodAt(t_server_config &config, Path path, Method method)
{
	while (true)
	{
		std::cout << "Attempting to find method " << method << " at " << path << std::endl;

		t_location location = get_location(config, path.asFilePath());
		if (location.root_dir == "/" || location.root_dir.empty())
			break;
		auto methodPair = location.allowed_methods.find(method);
		if (methodPair != location.allowed_methods.end())
			return methodPair->second;

		path.goUpOneDir();
		if (path.isRoot())
			break;
	};
	//TODO: just for info there might be more then one location so i think it makes sense to make it more dynamic
	auto foundLoc = config.locations[0].allowed_methods.find(method);
	if (foundLoc != config.locations[0].allowed_methods.end())
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
	urlPath.replace(0, config.root_dir.size(), "");
	if (urlPath == "")
		urlPath = "/";
	return urlPath;
}

// Gets the location config of the path, respecting subdirs
// TODO: please check this!!
t_location get_location(t_server_config &config, std::string path)
{
	// t_location loc = EMPTY_LOCATION; //what are u doing here @freddy?
	t_location loc;

	std::cout << "Getting location for path: \"" << path << "\"" << " at root " << config.root_dir << std::endl;
	if (path == config.root_dir)
	{
		std::cout << "The path is \"/\"." << std::endl;
		if (std::filesystem::exists(config.root_dir))
		{
			loc = config.locations[0]; //TODO: i changed it but im very sure that is not what you want
			std::cout << "Default location is valid." << std::endl;
		}
	}

	std::cout << "Initial location: " << loc.root_dir << std::endl;

	for (t_location &location : config.locations)
	{
		std::cout << "Current location: " << loc.root_dir << " checking against " << location.root_dir << std::endl;
		if (isSubroute(location.root_dir, path) && std::filesystem::exists(location.root_dir))
			if (loc.root_dir.size() < location.root_dir.size())
				loc = location;
	}

	return loc;
}

void setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("Failed to get socket flags");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set non-blocking mode");
}