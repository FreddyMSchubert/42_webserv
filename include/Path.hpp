#pragma once

#include <cstddef>
#include <string>
#include <filesystem>
#include <vector>

struct s_server_config typedef t_server_config;

class Path
{
	private:
		std::string		_path; // saved as URL type
		t_server_config	&_config;

	public:

		enum class Type
		{
			URL,
			FILESYSTEM
		};

		Path();
		Path(std::string path, Type type, t_server_config &config);
		~Path() = default;
		Path(const Path& other);
		Path& operator=(const Path& other);

		std::vector<std::filesystem::directory_entry> getDirectoryEntries();

		void goUpOneDir();
		void goDownIntoDir(const std::string& dir);

		bool empty() const;
		std::string path() const;
		std::string url() const;
		size_t size() const;

		void setConfig(t_server_config &config);

		static std::string combinePaths(const std::string& path1, const std::string& path2);

		std::string operator+(const std::string& other) const;
		bool operator==(const Path& other) const;
		bool operator!=(const Path& other) const;
};

std::ostream &operator<<(std::ostream &os, const Path &path);

