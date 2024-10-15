#pragma once

#include "Enums.hpp"
#include <cstddef>
#include <iostream>
#include <vector>

typedef struct s_location t_location;

typedef struct s_location 
{
	std::vector<std::pair<Method, OptionalBoolean>> allowed_methods;
	std::string root;
	std::string index;
	bool directory_listing;
	size_t client_max_body_size;
	bool autoindex;
	// std::vector<t_location> locations; // TODO: add this back later
	bool empty() { return root.empty(); }
} t_location;

inline std::ostream &operator<<(std::ostream &os, const t_location &location)
{
	os << "Root: " << location.root << std::endl;
	os << "Index: " << location.index << std::endl;
	os << "Directory listing: " << location.directory_listing << std::endl;
	os << "Client max body size: " << location.client_max_body_size << std::endl;
	os << "Autoindex: " << location.autoindex << std::endl;
	os << "Allowed methods: ";
	for (const std::pair<Method, OptionalBoolean> &noolean : location.allowed_methods)
		os << noolean.first << "(" << noolean.second << ")" << " ";
	os << std::endl;
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