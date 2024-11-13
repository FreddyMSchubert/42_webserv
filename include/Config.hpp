#pragma once

#include "Enums.hpp"
#include <cstddef>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <variant>
#include <regex>
#include <array>
#include <map>
#include "Path.hpp"
#include "Logger.hpp"
#include "FilePath.hpp"

// typedef struct s_location t_location;

typedef struct s_location
{
	std::variant<Path, FilePath> path;
	Path root_dir;
	std::unordered_map<Method, bool> allowed_methods;
	bool directory_listing;
	std::vector<std::string> cgi_extensions;
	std::map<int, Path> redirections;
	Path upload_dir;
} t_location;

class Config
{
	private:
		std::vector<std::string> _server_names;
		std::string _host;
		int _port;
		std::string _root_dir;
		FilePath _index_file;
		unsigned int _client_max_body_size; // in bytes
		std::map<int, FilePath> _error_pages;
		std::vector<t_location> _locations;

		void parseListen(std::string line);
		void parseServerName(std::string line);
		void parseRoot(std::string line);
		void parseIndex(std::string line);
		void parseClientMaxBodySize(std::string line);
		void parseErrorPage(std::string line);
		void parseLocation(std::string line);

	public:
		Config(std::string data);
		Config(const Config &other) = default;
		Config &operator=(const Config &other) = default;
		~Config() = default;
};
