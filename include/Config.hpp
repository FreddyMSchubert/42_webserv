#pragma once 

#include "Enums.hpp"
#include <cstddef>
#include <iostream>
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
#include "Settings.hpp"

#define CONFIG_KEYWORD_COUNT 8

typedef struct s_location
{
	std::variant<Path, FilePath>						path;				// path in config
	std::string											root_dir;			// folder to get data from. not a path as it might not be in config root. If empty, use config root
	std::array<bool, 3>									allowed_methods;	// saved in order of Method enum in Enums.hpp
	bool												directory_listing = false;
	std::vector<std::pair<std::string, std::string>>	cgi_extensions;
	std::map<int, Path>									redirections;
	Path												upload_dir;

	Path getPathAsPath() const { return std::holds_alternative<Path>(path) ? std::get<Path>(path) : static_cast<Path>(std::get<FilePath>(path)); }
} t_location;
std::ostream &operator<<(std::ostream &os, const t_location &loc);

class Config
{
	private:
		int serverId = -1;

		std::vector<std::string> _server_names;
		std::string _host;
		int _port;
		std::string _root_dir;
		std::optional<FilePath> _index_file; // optional but will always be present after constructor
		unsigned int _max_package_size = 1048576; // 1 MB in bytes
		std::map<int, FilePath> _error_pages;
		int _client_timeout = 30000; // 30 seconds in ms
		std::vector<t_location> _locations;

		// Line parsers
		void parseListen(const std::string & line);
		void parseServerName(const std::string & line);
		void parseRoot(const std::string & line);
		void parseIndex(const std::string & line);
		void parseMaxPackageSize(const std::string & line);
		void parseErrorPage(const std::string & line);
		void parseClientTimeout(const std::string & line);

		// Location Line Parsers
		void parseLocation(const std::string & line);
		void parseLocationRoot(const std::string & line, t_location & loc);
		void parseLocationAllowedMethods(const std::string & line, t_location & loc);
		void parseLocationAutoindex(const std::string & line, t_location & loc);
		void parseLocationCgiExtensions(const std::string &line, t_location &loc);
		void parseLocationRedirections(const std::string & line, t_location & loc);
		void parseLocationUploadDir(const std::string &line, t_location &loc);

		// Private Utils
		void extractConfigFromBrackets(std::vector<std::string> &lines, const std::string &data);

	public:
		Config();
		Config(std::string data, int serverId);
		Config(const Config &other) = default;
		Config &operator=(const Config &other) = default;
		~Config() = default;

		int getServerId() const { return serverId; }

		std::vector<std::string> getServerNames() const { return _server_names; }
		std::string getHost() const { return _host; }
		int getPort() const { return _port; }
		std::string getRootDir() const { return _root_dir; }
		FilePath getIndexFile() const { return *_index_file; }
		unsigned int getMaxPackageSize() const { return _max_package_size; }
		std::map<int, FilePath> getErrorPages() const { return _error_pages; }
		int getClientTimeout() const { return _client_timeout; }
		std::vector<t_location> getLocations() const { return _locations; }

		// Public Utils
		t_location getRootLocation() const;
};
