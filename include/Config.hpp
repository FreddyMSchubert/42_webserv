#ifndef CONFIG_HPP
#define CONFIG_HPP

#pragma once

#include "Enums.hpp"
#include "Path.hpp"
#include "FilePath.hpp"
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

// class FilePath;

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
	Path path;                                         // specifies the path of the location
	Path root_dir;                                    // specifies the root dir for the location
	std::unordered_map<Method, bool> allowed_methods;  // specifies the allowed Methods, <Method (GET, POST)><allowed(true, false)>
	bool directory_listing;                             // specifies if the directories should be shown or not
	std::vector<std::string> cgi_extensions;                 // specifies the cgi extensions as Path
	std::map<int, Path> redirections;                 // redirect to a new location (saved in Path) with the status code (saved in int)
	Path upload_dir;                                   // specifies the upload directory of this location
	bool empty() const { return path.isEmpty(); }
} t_location;

typedef struct s_server_config
{
	std::string host;                                   // specifies the host
	int port;                                     // specifies the port
	std::vector<std::string> server_names;         // specifies the server_names in a vector of Path
	std::string root_dir;                               // specifies the root directory for this server
	FilePath index_file;               				// specifies the index file
	size_t client_max_body_size;                  // specifies the client max body size in MB
	std::map<int, FilePath> error_pages;             // specifies what error pages should be displayed for what HTTP error code
	std::vector<t_location> locations;           // for more info look into t_location
	// t_location & default_location()
	// {
	// 	for (auto & loc : locations)
	// 		if (loc.path == "/")
	// 			return loc;
	// 	throw std::runtime_error("No default location found. Please include that!");
	// }
} t_server_config;

std::vector<t_server_config>    get_config(char *argv[], std::vector<t_server_config> & tmp_serv_conf);

#endif