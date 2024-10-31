#pragma once

#include <cstddef>
#include <string>
#include <filesystem>
#include <variant>
#include <vector>
#include "Config.hpp"

class FilePath;

struct s_server_config typedef t_server_config;

class Path
{
	protected:
		std::string		_path; // saved as URL type
		t_server_config	*_config;

	public:

		enum class Type
		{
			URL,
			FILESYSTEM
		};

		Path() : _path("/"), _config(nullptr) {};
		Path(std::string path, Type type, t_server_config &config);
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
		static std::variant<Path, FilePath> createPath(const std::string &path, Path::Type type, t_server_config *config);

		std::string operator+(const std::string& other) const;
		bool operator==(const Path& other) const;
		bool operator!=(const Path& other) const;

		void hardSetPath(const std::string& path);
		std::string getPath() const;
		void setConfig(t_server_config &config);
		t_server_config *getConfig() const;
};

std::ostream &operator<<(std::ostream &os, const Path &path);

#include "FilePath.hpp"