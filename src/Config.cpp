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

bool	start_of_a_server_r_block(const std::string& str, const std::string& regex)
{
	std::regex pattern(regex);
	return (std::regex_match(str, pattern));
}

int		end_of_a_server_r_block(std::vector<std::string> conf, size_t i)
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
		if (start_of_a_server_r_block(conf[i], R"(^\s*server\s*\{\s*$)") == true)
		{
			if (start_of_server == -1)
				start_of_server = i + 1;
			end_of_server = end_of_a_server_r_block(conf, i);
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


//why empty
void	init_location(std::vector<std::string> location_block, t_server_configs& server, int& iter)
{
	(void)location_block;
	(void)iter;
	(void)server;
}

std::string	replace_x_in_dir(int value, std::string& tmp_index)
{
	size_t pos = tmp_index.find('x');
	if (pos == std::string::npos)
		return (tmp_index);
	int	test = value % 10;
	char last_int = test + '0';
    if (pos != std::string::npos)
        tmp_index.replace(pos, 1, 1, last_int);
	return (tmp_index);
}

void	init_error_pages(std::string str, t_server_block& server, int& iter)
{
	std::regex pattern(R"(^\s*error_page\s+[1-5][0-9]{2}\s+/[1-5][0-9]{2}\.[a-zA-Z]+;\s*$)");
	(void)iter;
	if (std::regex_match(str, pattern) == false)
		parse_except("Expected 'error_page <HTTP response status codes> <directory>;' but didnt find it where it was expected! 'U SCUM' -Freddy");

	std::string tmp_error_pages = skip_until_value(str, "error_page");
	std::vector<std::string>	tmp_index;
	tmp_index = split(tmp_error_pages, ' ');
	tmp_index.erase(
	std::remove_if(tmp_index.begin(), tmp_index.end(),
					[](const std::string& name) {
						return name.empty() || std::all_of(name.begin(), name.end(), ::isspace);
					}),
	tmp_index.end());
	for (size_t i = 0; i <= tmp_index.size() - 1; i++)
	{
		std::string	str_for_atoi;
		for (const auto& c : tmp_index[i])
			if (std::isdigit(static_cast<unsigned char>(c)))
				str_for_atoi += c;
		int	atoi_value = std::atoi(str_for_atoi.c_str());
		server.error_pages.insert(std::make_pair(atoi_value, tmp_index[tmp_index.size() - 1]));
	}
}

void	init_client_max_body_size(std::string str, t_server_block& server, int& iter)
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

void	init_index_files(std::string str, t_server_block& server, int& iter)
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
                       return name.empty() || 
                              std::all_of(name.begin(), name.end(), [](char c) { 
                                  return std::isspace(c) || c == ';'; 
                              });
                   }),
    tmp_index.end());
	for (auto& name : tmp_index)
		name.erase(std::remove(name.begin(), name.end(), ';'), name.end());
	server.index_files = tmp_index;
}

void	init_root_dir(std::string str, t_server_block& server, int& iter)
{
	std::regex pattern(R"(^\s*root\s+(\./[^\s;]+)\s*;\s*$)");
	(void)iter;
	if (std::regex_match(str, pattern) == false)
		parse_except("Expected 'root <rootdirectory>;' but didnt find it where it was expected!");
	
	std::string tmp_root = skip_until_value(str, "root");
	int			i = 0;
	for (const auto& c : tmp_root)
		if (isalpha(static_cast<unsigned char>(c)) || c == '/' || c == '.')
			i++;
	server.root_dir = tmp_root.substr(0, i);
}

void	init_server_name(std::string str, t_server_block& server, int& iter)
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
	for (auto& name : names)
		name.erase(std::remove(name.begin(), name.end(), ';'), name.end());
	server.server_names = names;
}

void	init_port(std::string str, t_server_block& server, int& iter)
{
	std::regex only_port(R"(^\s*listen\s+([0-5]?\d{1,4}|6[0-5]\d{3}|66[0-5]\d{2}|66000)\s*;\s*$)");
	std::regex only_host(R"([\s\t]*listen[\s\t]*host\s*\(?\d{1,3}(?:\.\d{1,3}){3}\)?[\s\t]*;?)");
	std::regex both(R"(^\s*listen\s+([0-5]?\d{1,4}|6[0-5]\d{3}|66[0-5]\d{2}|66000)\s*;\s*$)");
	(void)iter;
	if (std::regex_match(str, only_port) == false && std::regex_match(str, only_host) == false && std::regex_match(str, both))
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
	if (std::atoi(str_for_atoi.c_str()) >= 0 && std::atoi(str_for_atoi.c_str()) <= 65535)
		server.port = std::atoi(str_for_atoi.c_str());
	else
		parse_except("Port is outside of the allowed range!");
}

void	init_allowed_methods(t_location& location, std::string str)
{
	std::string					pre_processed_methods = skip_until_value(str, "allowed_methods");
	std::vector<std::string>	post_processed_methods = split(pre_processed_methods, ' ');
	for (auto& method : post_processed_methods)
		method.erase(std::remove(method.begin(), method.end(), ';'), method.end());
	location.allowed_methods[Method::GET] = false;
	location.allowed_methods[Method::POST] = false;
	for (const auto& method : post_processed_methods)
		if (method.find("GET") != std::string::npos)
			location.allowed_methods[Method::GET] = true;
	for (const auto& method : post_processed_methods)
		if (method.find("POST") != std::string::npos)
			location.allowed_methods[Method::POST] = true;
}

void	init_redirection(t_location& location, std::string str)
{
	std::string					pre_processed_redirection = skip_until_value(str, "redirection");
	std::vector<std::string>	post_processed_redirection = split(pre_processed_redirection, ' ');
	for (auto& item : post_processed_redirection)
		item.erase(std::remove(item.begin(), item.end(), ';'), item.end());
	std::string					str_for_atoi;
	for (char c : post_processed_redirection[0])
	{
		if (std::isdigit(c))
			str_for_atoi += c;
		else
			break ;
	}
	location.redirections.insert(std::make_pair(std::atoi(str_for_atoi.c_str()), post_processed_redirection[post_processed_redirection.size() - 1]));		
}

void	init_root(t_location& location, std::string str)
{
	std::string pre_processed_root = skip_until_value(str, "root");
	int			i = 0;
	for (const auto& c : pre_processed_root)
		if (isalpha(static_cast<unsigned char>(c)) || c == '/' || c == '.')
			i++;
	location.root_dir = pre_processed_root.substr(0, i);
}

void	init_directory_listing(t_location& location, std::string str)
{
	std::string	preprocessed_autoindex = skip_until_value(str, "autoindex");
	
	if (str.find("on") != std::string::npos)
		location.directory_listing = true;
	else if (str.find("off") != std::string::npos)
		location.directory_listing = false;
	else
		parse_except("Autoindex does not have on or off!");
}

void	init_index(t_location& location, std::string str)
{
	std::string					tmp_str = skip_until_value(str, "index");
	std::vector<std::string>	tmp_index;
	tmp_index = split(tmp_str, ' ');
	tmp_index.erase(
    std::remove_if(tmp_index.begin(), tmp_index.end(),
                   [](const std::string& name) {
                       return name.empty() || 
                              std::all_of(name.begin(), name.end(), [](char c) { 
                                  return std::isspace(c) || c == ';'; 
                              });
                   }),
    tmp_index.end());
	for (auto& name : tmp_index)
		name.erase(std::remove(name.begin(), name.end(), ';'), name.end());
	location.index_files = tmp_index;
}

void	init_cgi_extention(t_location& location, std::string str)
{
	std::string					pre_processed_cgi = skip_until_value(str, "cgi_extension");
	std::vector<std::string>	splited_cgi = split(pre_processed_cgi, ' ');
	for (auto& str : splited_cgi)
		str.erase(std::remove(str.begin(), str.end(), ';'), str.end());
	location.cgi_extensions = splited_cgi;
}

void	init_upload_dir(t_location& location, std::string str)
{
	std::string pre_processed_root = skip_until_value(str, "upload_dir");
	int			i = 0;
	for (const auto& c : pre_processed_root)
		if (isalpha(static_cast<unsigned char>(c)) || c == '/' || c == '.')
			i++;
	location.root_dir = pre_processed_root.substr(0, i);
}


void	read_location(t_server_block& server, std::vector<std::string>& current_server, size_t start_of_location, size_t end_of_location, std::string location_str)
{
	t_location location;
	std::string loc = skip_until_value(location_str, "location");
	loc.erase(std::remove(loc.begin(), loc.end(), '{'), loc.end());
	loc.erase(std::remove(loc.begin(), loc.end(), ' '), loc.end());
	location.path = loc;

	std::regex allowed_methods(R"(^\s*allowed_methods\s+([A-Z]+\s*){1,2};\s*$)");
	std::regex redirection(R"(^\s*redirection\s+(3[0-5][0-9]|[1-4][0-9]{2}|5[0-9]{2})\s+\.\/[^\s;]+\s*;\s*$)");
	std::regex root(R"(^\s*root\s+(\./[^\s;]+)\s*;\s*$)");
	std::regex directory_listing(R"(^\s*autoindex\s+(on|off)\s*;\s*$)");
	std::regex index(R"(^\s*index\s+[^;]*;\s*$)");
	std::regex cgi_extension(R"(^\s*cgi_extension\s+\.\w+\s+\.\/[^\s;]+;\s*$)");
	std::regex upload_dir(R"(^\s*upload_dir\s+\./[^\s]*\s*;\s*$)");

	while (start_of_location < end_of_location && start_of_location < current_server.size())
	{
		if (std::regex_match(current_server[start_of_location], allowed_methods))
			init_allowed_methods(location, current_server[start_of_location]);
		else if (std::regex_match(current_server[start_of_location], redirection))
			init_redirection(location, current_server[start_of_location]);
		else if (std::regex_match(current_server[start_of_location], root))
			init_root(location, current_server[start_of_location]);
		else if (std::regex_match(current_server[start_of_location], directory_listing))
			init_directory_listing(location, current_server[start_of_location]);
		else if (std::regex_match(current_server[start_of_location], index))
			init_index(location, current_server[start_of_location]);
		else if (std::regex_match(current_server[start_of_location], cgi_extension))
			init_cgi_extention(location, current_server[start_of_location]);
		else if (std::regex_match(current_server[start_of_location], upload_dir))
			init_upload_dir(location, current_server[start_of_location]);
		else
			parse_except("Could not process: " + current_server[start_of_location] + " ! Please check the valid reference.");
		start_of_location++;
	}
	server.locations.push_back(location);
}

void fill_structs(std::vector<std::vector<std::string>>& preprocessed_servers, t_server_configs& tmp_serv_conf)
{
    std::vector<std::function<void(std::string, t_server_block&, int&)>> init_functions = {
        init_port, init_server_name, init_root_dir, init_index_files,
        init_client_max_body_size, init_error_pages
    };

	std::regex location_regex(R"(^\s*location\s+/\S*\s*\{\s*$)");
	std::regex server_end(R"(^\s*\}\s*$)");
    for (size_t server_idx = 0; server_idx < preprocessed_servers.size(); ++server_idx)
    {
        t_server_block	server;
        std::vector<std::string>& current_server = preprocessed_servers[server_idx];
        int counter = 0;

        for (size_t i = 0; i < current_server.size();)
        {
            std::string& line = current_server[i];
			if (std::regex_match(line, server_end) == true)
			{
				i++;
				continue;
			}
            if (i < init_functions.size())
            {
                init_functions[i](line, server, counter);
                ++counter;
                ++i;
                continue;
            }
            if (!std::regex_match(line, location_regex) && !check_if_search_str_in_src_str(line, "}"))
            {
                init_functions.back()(line, server, counter);
                ++counter;
                ++i;
                continue;
            }
			if (std::regex_match(line, location_regex))
			{
				int bracket_counter = 0;
				std::string location_str;
				if (check_if_search_str_in_src_str(line, "}") == false)
					location_str = line;
				size_t start_of_location = i + 1;
				while (i < current_server.size())
				{
					if (line.find('{') != std::string::npos)
						bracket_counter++;
					if (line.find('}') != std::string::npos)
						bracket_counter--;
					if (bracket_counter == 0)
						break;
					i++;
					line = current_server[i];
				}
				if (bracket_counter > 0)
					parse_except("Unexpected end of location, expecting '}' in config file.");	
				size_t	end_of_location = i;
				i++;
				read_location(server, current_server, start_of_location, end_of_location, location_str);
			}
        }
        tmp_serv_conf.push_back(server);
    }
}

t_server_configs	get_config(char *argv[])
{
	t_server_configs						tmp_serv_conf;
	std::vector<std::vector<std::string>>	preprocessed_servers;

	if (!argv[1])
		split_conf_in_server(preprocessed_servers, read_file("config/default.conf"));
	else
		split_conf_in_server(preprocessed_servers, read_file(argv[1]));
	fill_structs(preprocessed_servers, tmp_serv_conf);
	return (tmp_serv_conf);
}
