#pragma once

#include "Path.hpp"
class Config;

#include <fstream>
#include <sstream>
#include <variant>

class FilePath : public Path
{
	private:
		std::string _file;

	public:
		FilePath(const std::string &path, Path::Type type, Config &config);
		~FilePath() = default;

		std::string getFileContents() const;
		std::string getFileName() const;
		std::string getFileExtension() const;
		std::string getMimeType() const;
};

#include "Config.hpp"
