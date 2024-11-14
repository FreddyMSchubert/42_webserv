#pragma once

#include <cstddef>
#include <string>
#include <filesystem>
#include <variant>
#include <vector>
#include <optional>

class FilePath;

struct s_server_config typedef Config;

class Path
{
	protected:
		std::string		_path; // saved as URL type
		Config &		_config;

	public:
		enum class Type
		{
			URL,
			FILESYSTEM
		};

		Path(std::string path, Type type, Config &config);
		~Path() = default;
		Path(const Path& other)  : _path(std::string(other._path)), _config(other._config) {};
		Path& operator=(const Path& other);

		std::vector<std::filesystem::directory_entry> getDirectoryEntries();

		void goUpOneDir();
		void goDownIntoDir(const std::string& dir);

		bool isEmpty() const;
		bool isRoot() const;
		std::string asFilePath() const;
		std::string asUrl() const;
		size_t size() const;


		static std::string combinePaths(const std::string& path1, const std::string& path2);
		static std::variant<Path, FilePath> createPath(const std::string &path, Path::Type type, Config *config);
		static std::string verifyPath(std::string path);

		std::string operator+(const std::string& other) const;
		bool operator==(const Path& other) const;
		bool operator!=(const Path& other) const;

		void hardSetPath(const std::string& path);
		std::string getPath() const;
		void setConfig(Config &config);
		Config &getConfig() const;
};

std::ostream &operator<<(std::ostream &os, const Path &path);

#include "FilePath.hpp"