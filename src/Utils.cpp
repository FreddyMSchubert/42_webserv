#include "Utils.hpp"

bool isAllowedMethodAt(Config &config, Path path, Method method)
{
	while (true)
	{
		std::cout << "Attempting to find method " << method << " at " << path << std::endl;

		t_location location = get_location(config, path.asFilePath());
		std::cout << "Successfully got that location" << std::endl;
		if (location.root_dir.asUrl() == "/" || location.empty())
			break;
		auto methodPair = location.allowed_methods.find(method);
		if (methodPair != location.allowed_methods.end())
			return methodPair->second;

		path.goUpOneDir();
		if (path.isRoot())
			break;
	};

	const t_location& rootLoc = config.getRootLocation();
	auto foundLoc = rootLoc.allowed_methods.find(method);
	if (foundLoc != rootLoc.allowed_methods.end())
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

std::string getFilePathAsURLPath(std::string path, Config &config)
{
	std::string urlPath = path;
	urlPath.replace(0, config.getRootLocation().root_dir.size(), "");
	if (urlPath == "")
		urlPath = "/";
	return urlPath;
}

// Gets the location config of the path, respecting subdirs
t_location get_location(Config &config, std::string path)
{
	t_location loc = config.getRootLocation();

	std::cout << "Getting location for path: \"" << path << "\"" << " at root " << config.getRootDir() << std::endl;
	if (path == config.getRootLocation().root_dir.asUrl())
	{
		std::cout << "The path is \"/\"." << std::endl;
		if (std::filesystem::exists(config.getRootLocation().root_dir.asFilePath()))
		{
			loc = config.getRootLocation();
			std::cout << "Default location is valid." << std::endl;
		}
	}

	std::cout << "Initial location: " << loc.root_dir << std::endl;

	for (t_location &location : config.getLocations())
	{
		std::cout << "Current location: " << loc.root_dir << " checking against " << location.root_dir << std::endl;
		if (isSubroute(location.root_dir.asUrl(), path))
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

std::variant<Path, FilePath> createPath(const std::string &path, Path::Type type, Config &config)
{
	std::string filePath = path;
	if (type == Path::Type::URL)
		filePath = Path::combinePaths(config.getRootDir(), path);
	if (!std::filesystem::exists("." + filePath))
		throw std::runtime_error("Path ." + filePath + " does not exist");
	if (std::filesystem::is_regular_file("." + filePath))
		return FilePath(path, type, config);
	return Path(path, type, config);
}
