#pragma once

#include <string>
#include <filesystem>

#include "Config.hpp"

class Path
{
	private:
		std::string		_path; // saved as URL type
		t_server_config	_config;

	public:
		enum class Type
		{
			FILESYSTEM,
			URL
		};

		Path(const std::string& path, Type type, t_server_config config);
		~Path() = default;
		Path(const Path& other) = default;
		Path& operator=(const Path& other) = default;

		std::string getPathAs(const Type& type) const;
		std::vector<std::filesystem::directory_entry> getDirectoryEntries();

		void goUpOneDir();
		void goDownIntoDir(const std::string& dir);

};

std::string combinePaths(const std::string& path1, const std::string& path2);
