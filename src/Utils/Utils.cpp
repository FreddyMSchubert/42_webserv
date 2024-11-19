#include "Utils.hpp"

bool isAllowedMethodAt(Config &config, Path path, Method method)
{
	t_location location = get_location(config, path.asUrl());
	std::cout << "Found method: " << location << std::endl;
	return location.allowed_methods[static_cast<int>(method)];
}

std::vector<std::filesystem::directory_entry> getDirectoryEntries(const std::string& path)
{
	std::vector<std::filesystem::directory_entry> entries;
	for (const auto& entry : std::filesystem::directory_iterator(path))
		entries.push_back(entry);
	return entries;
}

std::string getFilePathAsURLPath(std::string path, Config &config)
{
	std::string urlPath = path;
	urlPath.replace(0, config.getRootLocation().root_dir.size(), "");
	if (urlPath == "")
		urlPath = "/";
	return urlPath;
}

// Gets the most specific config location of the path, respecting subdirs
// Expects path in URL format
// TODO: Make this consider files.
t_location get_location(Config &config, std::string path)
{
	t_location retLoc = config.getRootLocation();

	if (std::filesystem::is_regular_file("." + path))
		throw std::runtime_error("get_location can currently not yet handle files");

	if (path == config.getRootDir())
		return retLoc;

	for (t_location &currLoc : config.getLocations())
	{
		std::string currLocPath = std::holds_alternative<Path>(currLoc.path) ? std::get<Path>(currLoc.path).asUrl() : std::get<FilePath>(currLoc.path).asUrl();
		std::string retLocPath = std::holds_alternative<Path>(retLoc.path) ? std::get<Path>(retLoc.path).asUrl() : std::get<FilePath>(retLoc.path).asUrl();

		std::cout << "Comparing urrLocPath " << currLocPath << " with path " << path << " and retLocPath " << retLocPath << std::endl;

		if (path.find(currLocPath) == 0) // path is a subdir of location
			if (retLocPath.size() < currLocPath.size()) // is it more specific?
				retLoc = currLoc;
	}

	return retLoc;
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
