#include "Path.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>

Path::Path(std::string path, Type type, t_server_config *config)
{
	if (!config)
		throw std::runtime_error("Path: config is nullptr");
	_config = config;

	if (path.find("./") != std::string::npos)
		throw std::runtime_error("Path: Path contains ./ : " + path);
	if (path.find_last_of('/') != path.size() - 1)
		throw std::runtime_error("Path: Path does not end with / : " + path);
	if (_path.std::find_first_of('/') != 0) // FIXME: this is weird becuase it flags the path as not starting with a / even if it clearly does
	{
		for (char &c : path)
			std::cout << c;
		std::cout << std::endl;
		for (long unsigned int i = 0; i < _path.length(); i++)
			std::cout << _path[i] << std::endl;
		std::cout << std::endl;
		std::cout << _path.find_first_of('/') << std::endl;
		std::cout << std::string::npos << std::endl;
		throw std::runtime_error("Path: Path does not start with / : " + path);
	}

	if (type == Type::URL)
		_path = path;
	else
	{
		// Convert filesystem path to URL
		if (path.find(_config->default_location.root.path()) != 0)
			throw std::runtime_error("Path: Path is not in the configs root directory");
		_path = path.substr(_config->default_location.root.size(), path.size());
	}

	if (!std::filesystem::exists(Path::combinePaths(_config->default_location.root.path(), _path)))
		throw std::runtime_error("Path: Path does not exist");
}

std::string Path::url() const
{
	return _path;
}

std::string Path::path() const
{
	if (!_config)
		throw std::runtime_error("Path: path: config is nullptr");
	return combinePaths(_config->default_location.root.path(), _path);
}

std::vector<std::filesystem::directory_entry> Path::getDirectoryEntries()
{
	std::vector<std::filesystem::directory_entry> entries;
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path()))
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
	if (!_config)
		throw std::runtime_error("Path: goDownIntoDir: config is nullptr");

	if (dir.find('/') != std::string::npos)
		throw std::runtime_error("Path: goDownIntoDir: dir contains /");

	std::string new_path = _path + dir + (dir.back() == '/' ? "" : "/");
	std::string new_path_filesystem = Path::combinePaths(_config->default_location.root.path(), new_path);
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

void Path::setConfig(t_server_config *config)
{
	_config = config;
	if (!_config)
		Logger::Log(LogLevel::WARNING, "Path: setConfig: config set as nullptr");
}

Path::Path(const Path& other)
{
	_path = std::string(other._path);
	_config = other._config;
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
	return (os << path.url());
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

Path::Path() : _path(""), _config(nullptr) { }

size_t Path::size() const
{
	return _path.size();
}

