#include "Path.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>

// Path class shall never represent a non-present path

Path::Path(std::string path, Type type, t_server_config *config) : _config(config)
{
	std::cout << "Path constructor called with path: " << path << " and type: " << (type == Path::Type::FILESYSTEM) << std::endl;

	if (path.find("./") != std::string::npos)
		throw std::runtime_error("Path: Path contains ./ : " + path);
	if (path.find_last_of('/') != path.size() - 1)
		path.push_back('/');
	if (path.find_first_of("/") != 0) // FIXME: this is weird because it flags the path as not starting with a / even if it clearly does
		path = "/" + path;

	if (type == Type::FILESYSTEM)
		_path = path;
	else
	{
		// Convert filesystem path to URL
		if (!_config)
			throw std::runtime_error("Path: Path: config is nullptr");
		_path = Path::combinePaths(_config->root_dir, path);
	}

	if (!std::filesystem::exists(_path))
		throw std::runtime_error("Path: Path does not exist");
}

std::string Path::asUrl() const
{
	if (_path.find(_config->root_dir) != 0)
		throw std::runtime_error("Path: asUrl: path does not start with root_dir");
	return _path.substr(_config->root_dir.size());
}

std::string Path::asFilePath() const
{
	return _path;
}

std::vector<std::filesystem::directory_entry> Path::getDirectoryEntries()
{
	std::vector<std::filesystem::directory_entry> entries;
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(this->asFilePath()))
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
	if (!std::filesystem::exists(new_path))
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
std::variant<Path, FilePath> Path::createPath(const std::string &path, Path::Type type, t_server_config *config)
{
	std::string filePath = path;
	if (type == Path::Type::URL)
		filePath = Path::combinePaths(config->root_dir, path);
	if (!std::filesystem::exists(filePath))
		throw std::runtime_error("Path does not exist");
	if (std::filesystem::is_regular_file(filePath))
		return FilePath(path, type, config);
	return Path(path, type, config);
}

Path& Path::operator=(const Path& other)
{
	if (this == &other)
		return *this;
	_path = other._path;
	_config = other._config;
	if (!_config)
		Logger::Log(LogLevel::WARNING, "Path: operator=: config set as nullptr");
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
bool Path::isSubrouteOf(Path & other) const
{
	return _path.find(other._path) == 0;
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
void Path::setConfig(t_server_config &config)
{
	_config = &config;
	if (!_config)
		Logger::Log(LogLevel::WARNING, "Path: setConfig: config set as nullptr");
}
t_server_config *Path::getConfig() const
{
	return _config;
}
