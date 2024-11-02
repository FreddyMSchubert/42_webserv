#pragma once

#include "Enums.hpp"
#include "Path.hpp"
#include "FilePath.hpp"
#include "Path.hpp"
#include "Logger.hpp"
#include <cstddef>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <fstream>
#include <regex>
#include <exception>
#include <filesystem>

class	Parsing_Exception : public std::exception
{
	private:
		std::string error_message;

		const std::string example_conf = R"(server {
    listen 127.0.0.1:4242;
    server_name example.com yourumu.de;

    root ./var/www/html;
    index index.html index.php index.deinemutter;

    client_max_body_size 10MB;

    error_page 404 /404.html;
    error_page 418 /418.html;

    location /example_route {
    root ./tmp/www;
    allowed_methods GET POST; 
    autoindex on;
    index index.html index.php;
    cgi_extension .php ./path/to/php-cgi;
    redirection 301 ./new-location;
    upload_dir ./var/www/uploads;
    }
	})";

	public:

		Parsing_Exception(const std::string& message) : error_message(message) {}

		const char* what() const noexcept override
		{
			return (error_message.c_str());
		}

		const std::string& get_example_conf() const
		{
			return (example_conf);
		}
};

typedef struct s_location
{
	std::filesystem::path					path;					// line 13: specifies the path of the location ./example_route
	std::filesystem::path					root_dir;				// line 14: specifies the root dir for the location
	std::unordered_map<Method, bool>		allowed_methods;		// line 15: specifies the allowed Methods, <Method (GET, POST)><allowed(true, false)>
	bool									directory_listing;		// line 16: specifies if the directorys should be shown or not
	std::vector<std::filesystem::path>		index_files;			// line 17: specifies the index files, each index file is one string in the vector
	std::vector<std::filesystem::path>		cgi_extensions;			// line 18: specifies the cgi extentions. Argument 1 is telling the server to handle files with example .php as cgi scripts. Argument 2 defines the path to the php executable 
	std::map<int, std::filesystem::path>	redirections;			// line 19: no error handling needed just redirect to a new location (which is saved in the string) with the status code (which is saved in the int)
	std::filesystem::path					upload_dir;				// line 20: specifies the upload directory of this location
	bool empty() const { return path.empty(); }
}   t_location;

typedef struct s_server_config
{
	std::string								host;					// line 2: specifies the host
	int										port;					// line 2: specifies the port
	std::vector<std::string> 				server_names;			// line 3: specifies the server_names in a vector of strings
	std::filesystem::path					root_dir;				// line 4: specifies the root directory for this server
	std::vector<std::filesystem::path>		index_files;			// line 5: specifies the index files, each index file is one string in the vector
	size_t									client_max_body_size;	// line 6: specifies the client max body size in MB
	std::map<int, std::filesystem::path>	error_pages;			// line 10/11: specifies what errorpages should be displayed for what http error code
	std::vector<t_location>					locations;				// line 13-21: for more info look into t_location
	t_location & default_location()
	{
		for (auto & loc : locations)
			if (loc.path == "/")
				return loc;
		throw std::runtime_error("No default location found. Please include that!");
	}
}	t_server_config;

std::vector<t_server_config>    get_config(char *argv[]);
