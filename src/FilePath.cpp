#include "Config.hpp"
#include "FilePath.hpp"

#include "./Packets/Methods/mimetypes.cpp"

FilePath::FilePath(const std::string &path, Path::Type type, t_server_config *config)
{
	std::string filePath = path;
	if (type == Path::Type::URL)
		filePath = Path::combinePaths(config->root_dir, path);

	int lastSlash = filePath.find_last_of('/');
	std::string folder = filePath.substr(0, lastSlash + 1);
	std::string file = filePath.substr(lastSlash + 1);

	if (!std::filesystem::exists(filePath))
		throw std::runtime_error("File does not exist");
	if (!std::filesystem::is_regular_file(filePath))
		throw std::runtime_error("Path is not a file");
	_path = folder;
	_file = file;
	_config = config;
}

std::string FilePath::getFileContents() const
{
	std::string filepath = Path::combinePaths(_path, _file);
	if (!std::filesystem::exists(filepath))
		throw std::runtime_error("File does not exist");
	if (!std::filesystem::is_regular_file(filepath))
		throw std::runtime_error("Path is not a file");
	std::ifstream file(filepath);
	if (!file.is_open() || !file.good())
		throw std::runtime_error("Failed to open file");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
std::vector<std::string> FilePath::getFileContentsListed() const
{
	std::string filepath = Path::combinePaths(_path, _file);
	if (!std::filesystem::exists(filepath))
		throw std::runtime_error("File does not exist");
	if (!std::filesystem::is_regular_file(filepath))
		throw std::runtime_error("Path is not a file");
	std::ifstream file(filepath);
	if (!file.is_open() || !file.good())
		throw std::runtime_error("Failed to open file");
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line))
		lines.push_back(line);
	return lines;
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
