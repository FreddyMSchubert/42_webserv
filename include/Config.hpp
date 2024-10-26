#pragma once

#include "Enums.hpp"
#include <cstddef>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <fstream>
#include <regex>
#include <exception>

class	Parsing_Exception : public std::exception
{
	private:
		std::string error_message;

		const std::string example_conf = R"(server {
		listen 80 default_server;
		server_name _;

		root /var/www/html;
		index index.html;

		client_max_body_size 10M;

		error_page 418	/418.php;

		location /example_route {

		allowed_methods GET POST; 

		redirection 301 ./new-location;

		root ./tmp/www;

		autoindex on;

		index index.html;

		cgi_extension .php ./path/to/php-cgi;

		upload_dir ./var/www/uploads;
		
		max_body_size 10MB;
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

typedef struct s_llocation
{
	std::string					path;					// The URI path for the location (e.g., "/", "/upload", "/kapouet")
	std::string					root_dir;				// The root directory for file search (e.g., /var/www/html or /tmp/www)
	std::vector<std::string>	allowed_methods;		// List of accepted HTTP methods (e.g., {"GET", "POST"})
	bool						autoindex;				// Flag to enable or disable directory listing (e.g., true/false)
	std::vector<std::string>	index_files;				// Default file to serve if the path is a directory (e.g., "index.html")
	std::vector<std::string>	cgi_extensions;			// Map of file extensions and the corresponding handler (e.g., ".php" -> "/path/to/php-cgi")
	std::string					cgi_pass;				// CGI handler or FastCGI socket (e.g., "unix:/var/run/php/php-fpm.sock")
	std::map<int, std::string>	redirections;			// URL for redirection if applicable (e.g., "/new-path") HTTP status code for redirection (e.g., 301)
	size_t						max_body_size;			// Maximum body size for uploads (e.g., 10MB)
	std::string					upload_dir;				// Directory to save uploaded files (e.g., "/var/www/uploads")
	bool						limit_except_enabled;	// Flag to enable/disable method restriction (e.g., true/false)
	bool empty() const
	{
		return path.empty() && 
			root_dir.empty() && 
			allowed_methods.empty() && 
			!autoindex && 
			index_files.empty() && 
			cgi_extensions.empty() && 
			cgi_pass.empty() && 
			redirections.empty() && 
			max_body_size == 0 && 
			upload_dir.empty() && 
			!limit_except_enabled;
	}
}   t_llocation;

typedef struct s_sserver
{
	int							port;
	std::vector<std::string> 	server_names;
	std::string					root_dir;
	std::vector<std::string>	index_files;
	size_t						client_max_body_size;
	std::map<int, std::string>	error_pages;
	std::vector<t_llocation>	locations;
}	t_sserver;

typedef struct s_sserver_configs
{
	std::vector<t_sserver> server_list;
}	t_sserver_configs;


t_sserver_configs    get_config(char *argv[]);

//----------------------------------------------------------------

// typedef struct s_location t_location;

typedef struct s_location 
{
	std::unordered_map<Method, bool> allowed_methods;
	std::string root; // needs to be relative to project root & start with . otherwise mayhem
	std::string index;
	bool directory_listing;
	size_t client_max_body_size;
	bool empty() { return root.empty(); }
} t_location;
#define EMPTY_LOCATION (t_location){{}, "", "", false, 0}

// inline std::ostream &operator<<(std::ostream &os, const t_location &location)
// {
// 	os << "Root: " << location.root << std::endl;
// 	os << "Index: " << location.index << std::endl;
// 	os << "Directory listing: " << location.directory_listing << std::endl;
// 	os << "Client max body size: " << location.client_max_body_size << std::endl;
// 	os << "Allowed methods: ";
// 	for (auto &noolean : location.allowed_methods)
// 		os << noolean.first << "(" << noolean.second << ")" << " ";
// 	os << std::endl;
// 	return os;
// }
inline std::ostream &operator<<(std::ostream &os, const t_location &location)
{
	os << "Root: " << location.root << std::endl;
	os << "Index: " << location.index << std::endl;
	os << "Directory listing: " << location.directory_listing << std::endl;
	os << "Client max body size: " << location.client_max_body_size << std::endl;
	os << "Allowed methods: ";
	for (auto &method : location.allowed_methods)
		os << method.first << "(" << method.second << ")" << " ";
	return os;
}

typedef struct s_error_page
{
	int error_code;
	t_location path;
} t_error_page;

typedef struct s_server_config
{
	std::vector<std::string> server_names;
	std::string host;
	int port; // below 1024 require root

	t_location default_location;
	std::vector<t_location> locations;
	std::vector<t_error_page> error_pages;

} t_server_config;
