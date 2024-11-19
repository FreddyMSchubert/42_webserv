#include "Path.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>

// Path class shall never represent a non-present path
// It also only exists in context of a config. No config, no path

Path::Path(std::string path, Type type, Config &config) : _config(config)
{
	// std::cout << "Path constructor called with path: " << path << " and type: " << (type == Path::Type::FILESYSTEM ? "Path::Type::Filesystem" : "Path::Type::URL") << " and config root " << config.getRootDir() << std::endl;

	std::string fspath;
	if (type == Path::Type::FILESYSTEM)
		fspath = verifyPath(path);
	else
		fspath = verifyPath(Path::combinePaths(_config.getRootDir(), path));
	
	if (fspath.find(_config.getRootDir()) != 0)
		throw std::runtime_error("Path: Path does not start with root dir: " + fspath);

	_path = fspath.substr(_config.getRootDir().size());

	if (_path.empty())
		_path = "/";
}

std::string Path::asUrl() const
{
	return _path;
}

std::string Path::asFilePath() const
{
	return combinePaths(_config.getRootDir(), _path);
}

std::vector<std::filesystem::directory_entry> Path::getDirectoryEntries()
{
	std::vector<std::filesystem::directory_entry> entries;
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator("." + this->asFilePath()))
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
	std::string new_path_filesystem = Path::combinePaths(_config.getRootLocation().root_dir, new_path);
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
/*
	Expects filesystem type
	Returns validated path
*/
std::string Path::verifyPath(std::string path)
{
	if (path.find("./") != std::string::npos)
		throw std::runtime_error("Path: Path contains ./ : " + path);
	if (path.find_last_of('/') != path.size() - 1)
		path.push_back('/');
	if (path.find_first_of('/') != 0)
		path = "/" + path;
	if (!std::filesystem::exists("." + path))
		throw std::runtime_error("Path: Path ." + path + " does not exist");
	return path;
}

Path& Path::operator=(const Path& other)
{
	if (this == &other)
		return *this;
	_path = other._path;
	_config = other._config;
	return *this;
}

std::ostream &operator<<(std::ostream &os, const Path &path)
{
	return (os << path.asUrl());
}

std::string Path::operator+(const std::string& other) const
{
	return _path + other;
}

bool Path::isEmpty() const
{
	return _path.empty();
}
bool Path::isRoot() const
{
	return _path == "/" || _path.empty();
}

bool Path::operator==(const Path& other) const
{
	return _path == other._path;
}
bool Path::operator!=(const Path& other) const
{
	return _path != other._path;
}

size_t Path::size() const
{
	return _path.size();
}

void Path::hardSetPath(const std::string& path)
{
	_path = path;
}
std::string Path::getPath() const
{
	return _path;
}
void Path::setConfig(Config &config)
{
	_config = config;
}
Config &Path::getConfig() const
{
	return _config;
}
