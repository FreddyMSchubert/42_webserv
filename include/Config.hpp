#pragma once

#include "Enums.hpp"
#include <cstddef>
#include <iostream>
#include <unordered_map>
#include <optional>
#include <vector>
#include <string>
#include <variant>
#include <regex>
#include <array>
#include <map>
#include "Path.hpp"
#include "Logger.hpp"
#include "FilePath.hpp"

typedef struct s_location
{
	std::variant<Path, FilePath> path;
	Path root_dir;
	std::unordered_map<Method, bool> allowed_methods;
	bool directory_listing;
	std::vector<std::string> cgi_extensions;
	std::map<int, Path> redirections;
	Path upload_dir;
	bool empty() const { return path.index() == 0; }
} t_location;

class Config
{
	private:
		std::vector<std::string> _server_names;
		std::string _host;
		int _port;
		std::string _root_dir;
		std::optional<FilePath> _index_file; // optional but will always be present after constructor
		unsigned int _client_max_body_size; // in bytes
		std::map<int, FilePath> _error_pages;
		std::vector<t_location> _locations;

		void parseListen(const std::string & line);
		void parseServerName(const std::string & line);
		void parseRoot(const std::string & line);
		void parseIndex(const std::string & line);
		void parseClientMaxBodySize(const std::string & line);
		void parseErrorPage(const std::string & line);
		void parseLocation(const std::string & line);

	public:
		Config(std::string data);
		Config(const Config &other) = default;
		Config &operator=(const Config &other) = default;
		~Config() = default;

		std::vector<std::string> getServerNames() const { return _server_names; }
		std::string getHost() const { return _host; }
		int getPort() const { return _port; }
		std::string getRootDir() const { return _root_dir; }
		FilePath getIndexFile() const { return *_index_file; }
		unsigned int getClientMaxBodySize() const { return _client_max_body_size; }
		std::map<int, FilePath> getErrorPages() const { return _error_pages; }
		std::string getErrorPage(int code) const;
		std::vector<t_location> getLocations() const { return _locations; }

		t_location getRootLocation() const;
};
