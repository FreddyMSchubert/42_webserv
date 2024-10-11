#pragma once

#include "Enums.hpp"
#include <cstddef>
#include <iostream>
#include <vector>

typedef struct s_location 
{
	std::vector<Method> allowed_methods;
	std::string root;
	std::string index;
	bool directory_listing;
	size_t client_max_body_size;
} t_location;

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