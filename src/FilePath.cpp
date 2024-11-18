#include "FilePath.hpp"

#include "./Packets/Methods/mimetypes.cpp"

// /error_pages/404.html -> /error_pages/
std::string formatPath(std::string path)
{
	if (path.find('/') != std::string::npos)
		path = path.substr(0, path.find_last_of('/') + 1);
	else
		path = "/";
	return path;
}

FilePath::FilePath(const std::string &path, Path::Type type, Config &config) : Path(formatPath(path), type, config)
{
	std::cout << "FilePath constructor called with path: " << path << " and type: " << (type == Path::Type::FILESYSTEM ? "Path::Type::Filesystem" : "Path::Type::URL") << " and config root " << config.getRootDir() << std::endl;

	std::string filePath = path;
	if (type == Path::Type::URL)
		filePath = Path::combinePaths(config.getRootDir(), path);

	int lastSlash = filePath.find_last_of('/');
	std::string file = filePath.substr(lastSlash + 1);

	if (!std::filesystem::exists("." + filePath))
		throw std::runtime_error("File \"" + filePath + "\" does not exist");
	if (!std::filesystem::is_regular_file("." + filePath))
		throw std::runtime_error("Path is not a file");
	_file = file;
	_config = config;
}

std::string FilePath::getFileContents() const
{
	if (!std::filesystem::exists("." + asFilePath()))
		throw std::runtime_error("File does not exist");
	if (!std::filesystem::is_regular_file("." + asFilePath()))
		throw std::runtime_error("Path is not a file");
	std::ifstream file("." + asFilePath());
	if (!file.is_open() || !file.good())
		throw std::runtime_error("Failed to open file");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
std::string FilePath::getFileName() const
{
	return _file;
}
std::string FilePath::getFileExtension() const
{
	if (_file.find('.') == std::string::npos)
		return "";
	return _file.substr(_file.find_last_of('.') + 1);
}
std::string FilePath::getMimeType() const
{
	std::string extension = getFileExtension();
	if (extension.empty())
		return DEFAULT_MIMETYPE;
	auto it = mimeTypes.find(extension);
	if (it == mimeTypes.end())
		return DEFAULT_MIMETYPE;
	return it->second;
}

std::string FilePath::asFilePath() const
{
	return Path::combinePaths(Path::combinePaths(_config.getRootDir(), _path), _file);
}
std::string FilePath::asUrl() const
{
	return Path::combinePaths(_path, _file);
}
