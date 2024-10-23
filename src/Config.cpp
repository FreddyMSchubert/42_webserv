#include "../include/Config.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <cctype>

std::vector<std::string> read_file(const std::string& path_to_conf)
{
	std::vector<std::string> result_str;
	std::ifstream conf(path_to_conf);
	std::string line;

	if (conf.is_open() == false || conf.good() == false)
		throw std::runtime_error(std::string("Couldn't read file at ") + path_to_conf);

	while (std::getline(conf, line))
		result_str.push_back(line);

	conf.close();
	return (result_str);
}

bool	check_if_search_str_in_src_str(const std::string& str, const std::string search)
{
	return (str.find_first_not_of(search) == std::string::npos);
}

bool	start_of_a_server(const std::string& str)
{
	std::regex pattern(R"(^\s*server\s*\{\s*$)");
	return (std::regex_match(str, pattern));
}

int		end_of_a_server(std::vector<std::string> conf, size_t i)
{
	int	bracket_counter = 0;

	while (i < conf.size())
	{
		if (conf[i].find('{') != std::string::npos)
			bracket_counter++;
		if (conf[i].find('}') != std::string::npos)
			bracket_counter--;
		if (bracket_counter == 0)
			break ;
		i++;
	}
	if (bracket_counter > 0)
		throw std::runtime_error(std::string("Unexpected end of file, expecting '}' in config file"));
	return (i);
}

void split_conf_in_server(std::vector<std::vector<std::string>>& preprocessed_servers, std::vector<std::string> conf)
{
	int start_of_server = -1;
	int end_of_server = -1;

	for (size_t i = 0; i < conf.size(); ++i)
	{
		if (check_if_search_str_in_src_str(" \t\n\r", conf[i]) == true)
			continue ;
		if (start_of_a_server(conf[i]) == true)
		{
			if (start_of_server == -1)
				start_of_server = i + 1;
			end_of_server = end_of_a_server(conf, i);
			i = end_of_server;
		}
		if (start_of_server != -1 && end_of_server != -1)
		{
			std::vector<std::string> server_string;

			while (start_of_server <= end_of_server)
			{
				if (check_if_search_str_in_src_str(conf[start_of_server], " \t\n\r"))
				{
					start_of_server++;
					continue ;
				}
				server_string.push_back(conf[start_of_server]);
				start_of_server++;
			}
			preprocessed_servers.push_back(server_string);
			start_of_server = -1;
			end_of_server = -1;
		}
	}
}

void	parse_except(std::string cause)
{
	Parsing_Exception exception(cause);

	std::cerr << exception.get_example_conf() << "\nTake this as a valid reference.\nError:" << std::endl;
	throw exception;
}

std::string skip_until_value(std::string& original_str, std::string skip)
{
    const char *c_original_str = original_str.c_str();
    
    while (*c_original_str && std::isspace(*c_original_str))
        ++c_original_str;
    if (std::strncmp(c_original_str, skip.c_str(), skip.length()) == 0)
        c_original_str += skip.length();
    while (*c_original_str && std::isspace(*c_original_str))
        ++c_original_str;
    size_t index = c_original_str - original_str.c_str();
    return (original_str.substr(index));
}

std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    size_t start = 0, end = 0;

    while ((end = str.find(delimiter, start)) != std::string::npos)
	{
        token = str.substr(start, end - start);
        tokens.push_back(token);
        start = end + 1;
    }
    
    tokens.push_back(str.substr(start));
    return (tokens);
}

void	init_location(std::vector<std::string> location_block, t_sserver& server, int& iter)
{
	(void)location_block;
	(void)iter;
	(void)server;
}

std::string	replace_x_in_dir(int value, std::string& tmp_index)
{
	char last_int = (value % 10) + 0;
	std::cout << "last_int: " << last_int << std::endl;
	size_t pos = tmp_index.find('x');
    if (pos != std::string::npos)
        tmp_index.replace(pos, 1, 1, last_int);
	std::cout << "tmp_index: " << tmp_index << std::endl;
	return (tmp_index);
}

void	init_error_pages(std::string str, t_sserver& server, int& iter)
{
	std::regex pattern(R"(^\s*error_page\s+(?:(?:[1-5][0-9]{2}\s+)*[1-5][0-9]{2}\s+)?/\d{2}[a-zA-Z]+\.([a-zA-Z]+);\s*$)");
	(void)iter;
	if (std::regex_match(str, pattern) == false)
		parse_except("Expected 'error_pages <HTTP response status codes> <directory>;' but didnt find it where it was expected!");

	std::string tmp_error_pages = skip_until_value(str, "error_page");
	std::vector<std::string>	tmp_index;
	tmp_index = split(tmp_error_pages, ' ');
	tmp_index.erase(
	std::remove_if(tmp_index.begin(), tmp_index.end(),
					[](const std::string& name) {
						return name.empty() || std::all_of(name.begin(), name.end(), ::isspace);
					}),
	tmp_index.end());
	for (size_t i = 0; i <= tmp_index.size(); i++)
		std::cout << tmp_index[i] << std::endl;
	int	last_item = tmp_index.size();
	for (size_t i = 0; i <= tmp_index.size() - 1; i++)
	{
		std::string	str_for_atoi;
		for (const auto& c : tmp_index[i])
		{
			if (std::isdigit(static_cast<unsigned char>(c)))
				str_for_atoi += c;
		}
		int	atoi_value = std::atoi(str_for_atoi.c_str());
		if (atoi_value >= 100 && atoi_value <= 199)
			server.error_pages.error_pages_100.insert(std::make_pair(atoi_value, replace_x_in_dir(atoi_value, tmp_index[last_item])));
		if (atoi_value >= 200 && atoi_value <= 299)
			server.error_pages.error_pages_200.insert(std::make_pair(atoi_value, replace_x_in_dir(atoi_value, tmp_index[last_item])));
		if (atoi_value >= 300 && atoi_value <= 399)
			server.error_pages.error_pages_300.insert(std::make_pair(atoi_value, replace_x_in_dir(atoi_value, tmp_index[last_item])));
		if (atoi_value >= 400 && atoi_value <= 499)
			server.error_pages.error_pages_400.insert(std::make_pair(atoi_value, replace_x_in_dir(atoi_value, tmp_index[last_item])));
		if (atoi_value >= 500 && atoi_value <= 599)
			server.error_pages.error_pages_500.insert(std::make_pair(atoi_value, replace_x_in_dir(atoi_value, tmp_index[last_item])));
	}
}

void	init_client_max_body_size(std::string str, t_sserver& server, int& iter)
{
	std::regex pattern(R"(^\s*client_max_body_size\s+[^;]*;\s*$)");

	if (std::regex_match(str, pattern) == false)
	{
		server.client_max_body_size = 1;
		iter--;
		return ;
	}
	std::string tmp_size = skip_until_value(str, "client_max_body_size");
	std::string	str_for_atoi;
	for (char c : tmp_size)
	{
		if (std::isdigit(c))
			str_for_atoi += c;
		else
			break ;
	}
	server.client_max_body_size = std::atoi(str_for_atoi.c_str());	
}

void	init_index_files(std::string str, t_sserver& server, int& iter)
{
	std::regex pattern(R"(^\s*index\s+[^;]*;\s*$)");
	(void)iter;
	if (std::regex_match(str, pattern) == false)
		parse_except("Expected 'index <indexfiles>;' but didnt find it where it was expected!");
	
	std::string					tmp_str = skip_until_value(str, "index");
	std::vector<std::string>	tmp_index;
	tmp_index = split(tmp_str, ' ');
	tmp_index.erase(
	std::remove_if(tmp_index.begin(), tmp_index.end(),
					[](const std::string& name) {
						return name.empty() || std::all_of(name.begin(), name.end(), ::isspace);
					}),
	tmp_index.end());
	server.index_files = tmp_index;
}

void	init_root_dir(std::string str, t_sserver& server, int& iter)
{
	std::regex pattern(R"(^\s*root\s+[^;]*;\s*$)");
	(void)iter;
	if (std::regex_match(str, pattern) == false)
		parse_except("Expected 'root <rootdirectory>;' but didnt find it where it was expected!");
	
	std::string tmp_root = skip_until_value(str, "root");
	int			i = 0;
	for (const auto& c : tmp_root)
		if (isalpha(static_cast<unsigned char>(c)) || c == '/')
			i++;
	server.root_dir = tmp_root.substr(i);
}

void	init_server_name(std::string str, t_sserver& server, int& iter)
{
	std::regex pattern(R"(^\s*server_name\s+[^;]*;\s*$)");
	(void)iter;
	if (std::regex_match(str, pattern) == false)
		parse_except("Expected 'server_name <variables>;' but didnt find it where it was expected!");
	
	std::string					tmp_str = skip_until_value(str, "server_name");
	std::vector<std::string>	names;
	names = split(tmp_str, ' ');
	names.erase(
	std::remove_if(names.begin(), names.end(),
					[](const std::string& name) {
						return name.empty() || std::all_of(name.begin(), name.end(), ::isspace);
					}),
	names.end());
	server.server_name = names;
}

void	init_port(std::string str, t_sserver& server, int& iter)
{
	std::regex pattern(R"(^\s*listen\s+([0-5]?\d{1,4}|6[0-5]\d{3}|66[0-5]\d{2}|66000)\s+default_server;\s*$)");
	(void)iter;
	if (std::regex_match(str, pattern) == false)
		parse_except("Expected 'listen <port> default_server' but didnt find it where it was expected!");

	std::string	tmp_str = skip_until_value(str, "listen");
	std::string str_for_atoi;
	for (char c : tmp_str)
	{
		if (std::isdigit(c))
			str_for_atoi += c;
		else
			break ;
	}
	server.port = std::atoi(str_for_atoi.c_str());
}

// need to code all the other function
//need location
void fill_structs(std::vector<std::vector<std::string>>& preprocessed_servers, t_sserver_configs& tmp_serv_conf)
{
	//need last function more than once
	std::vector<std::function<void(std::string, t_sserver&, int&)>> init_functions = {init_port, init_server_name, init_root_dir, init_index_files, init_client_max_body_size, init_error_pages};
	int	counter = 0;

	for (size_t server_idx = 0; server_idx < preprocessed_servers.size(); ++server_idx)
	{
		t_sserver server;

		std::vector<std::string>& current_server = preprocessed_servers[server_idx];

		for (size_t i = 0; i < current_server.size(); ++i)
		{
			std::string& line = current_server[counter];
			//i would be size and then thats it
			if (i < init_functions.size())
			{
				init_functions[i](line, server, counter);
				counter++;
				continue ;
			}
			//dunno if this works
			// if (!check_if_search_str_in_src_str(line, "location /") == true)
			// 	i -= 2;
			// if (check_if_search_str_in_src_str(line, "location /") == true)
			// {
			// 	// location();
			// 	// std::cout << "Found location block: " << line << std::endl;
			// 	continue;
			// }
		}
		tmp_serv_conf.server_list.push_back(server);
	}
}

t_sserver_configs	get_config(char *argv[])
{
	t_sserver_configs						tmp_serv_conf;
	std::vector<std::vector<std::string>>	preprocessed_servers;

	if (!argv[1])
		split_conf_in_server(preprocessed_servers, read_file("config/default.conf"));
	else
		split_conf_in_server(preprocessed_servers, read_file(argv[1]));
	fill_structs(preprocessed_servers, tmp_serv_conf);
	return (tmp_serv_conf);
}
