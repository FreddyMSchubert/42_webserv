#include "Utils.hpp"

std::vector<std::filesystem::directory_entry> getDirectoryEntries(const std::string& path)
{
	std::vector<std::filesystem::directory_entry> entries;
	for (const auto& entry : std::filesystem::directory_iterator(path))
		entries.push_back(entry);
	return entries;
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

		if (path.find(currLocPath) == 0) // path is a subdir of location
			if (retLocPath.size() < currLocPath.size()) // is it more specific?
				retLoc = currLoc;
	}

	return retLoc;
}

// returns a vector of all applying locations, in order of specificity
std::vector<t_location> get_locations(Config &config, std::string path)
{
	std::vector<t_location> retLocs;
	while (true)
	{
		t_location loc = get_location(config, path);
		if (retLocs.empty() || loc.path != retLocs.back().path)
			retLocs.push_back(loc);
		else
			break;

		std::string locPath = std::holds_alternative<Path>(loc.path) ? std::get<Path>(loc.path).asUrl() : std::get<FilePath>(loc.path).asUrl();
		path = path.substr(locPath.size());
		if (path[0] == '/')
			path = path.substr(1);
		if (path.empty())
			break;
	}
	return retLocs;
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

std::string getFileData(std::string path)
{
	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file " + path);
	std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return data;
}
