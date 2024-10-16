#include "Path.hpp"
#include "Config.hpp"

Path::Path(std::string path, Type type, t_server_config &config)
	: _path(""), _config(config)
{
	if (path.find("./") != std::string::npos)
		throw std::runtime_error("Path: Path contains ./");
	if (path.find_last_of('/') != path.size() - 1)
		throw std::runtime_error("Path: Path does not end with /");
	if (_path.find_first_of('/') != 0)
		throw std::runtime_error("Path: Path does not start with /");

	if (type == Type::URL)
		_path = path;
	else
	{
		// Convert filesystem path to URL
		if (path.find(_config.default_location.root) != 0)
			throw std::runtime_error("Path: Path is not in the configs root directory");
		_path = path.substr(_config.default_location.root.size(), path.size());
	}

	if (!std::filesystem::exists(Path::combinePaths(_config.default_location.root, _path)))
		throw std::runtime_error("Path: Path does not exist");
}

std::string Path::getPathAs(const Type& type) const
{
	if (type == Type::URL)
		return _path;
	else
		return _config.default_location.root + _path;
}

std::vector<std::filesystem::directory_entry> Path::getDirectoryEntries()
{
	std::vector<std::filesystem::directory_entry> entries;
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(getPathAs(Type::FILESYSTEM)))
		entries.push_back(entry);
	return entries;
}

void Path::goUpOneDir()
{
	if (_path.size() <= 1)
		return;

	_path.pop_back();
	_path = _path.substr(0, _path.find_last_of('/') + 1);
}

void Path::goDownIntoDir(const std::string& dir)
{
	if (dir.find('/') != std::string::npos)
		throw std::runtime_error("Path: goDownIntoDir: dir contains /");

	std::string new_path = _path + dir + (dir.back() == '/' ? "" : "/");
	std::string new_path_filesystem = Path::combinePaths(_config.default_location.root, new_path);
	if (!std::filesystem::exists(new_path_filesystem))
		throw std::runtime_error("Path: goDownIntoDir: Directory does not exist");
	_path = new_path;
}

// Combines two paths into one with a single / between them
std::string Path::combinePaths(const std::string& path1, const std::string& path2)
{
	if (path1.back() == '/' && path2.front() == '/')
		return path1 + path2.substr(1);
	else if (path1.back() != '/' && path2.front() != '/')
		return path1 + "/" + path2;
	else
		return path1 + path2;
}

Path::Path(const Path& other) : _path(other._path), _config(other._config) { }

Path& Path::operator=(const Path& other)
{
	if (this == &other)
		return *this;
	_path = other._path;
	_config = other._config;
	return *this;
}

size_t Path::size() const
{
	return _path.size();
}

std::ostream &operator<<(std::ostream &os, const Path &path)
{
	return (os << path.getPathAs(Path::Type::URL));
}

std::string Path::operator+(const std::string& other) const
{
	return _path + other;
}

bool Path::empty() const
{
	return _path.empty();
}

bool Path::operator==(const Path& other) const
{
	return _path == other._path;
}

bool Path::operator!=(const Path& other) const
{
	return _path != other._path;
}

Path::Path() : _path(""), _config({}) { }