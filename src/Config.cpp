#include "../include/Config.hpp"

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

void	init_location(std::vector<std::string> location_block, t_sserver& server)
{
	
}

void	init_error_pages(std::string str, t_sserver& server)
{
	(void)str;
	(void)server;
}

void	init_client_max_body_size(std::string str, t_sserver& server)
{
	(void)str;
	(void)server;
}

void	init_index_files(std::string str, t_sserver& server)
{
	(void)str;
	(void)server;
}

void	init_root_dir(std::string str, t_sserver& server)
{
	(void)str;
	(void)server;
}

void	init_server_name(std::string str, t_sserver& server)
{
	(void)str;
	(void)server;
}

void	init_port(std::string str, t_sserver& server)
{
	std::regex pattern(R"(^listen\s+(\d+)\s+default_server;\s*$)");
	std::cout << str << std::endl;
	if (std::regex_match(str, pattern) == false)
		parse_except("Expected 'listen <port> default' but didnt find it where it was expected!");
	
}

// need to code all the other function
//need location
//check docker again!
void fill_structs(std::vector<std::vector<std::string>>& preprocessed_servers, t_sserver_configs& tmp_serv_conf)
{
	std::vector<std::function<void(std::string, t_sserver&)>> init_functions = {init_port, init_server_name, init_root_dir, init_index_files, init_client_max_body_size, init_error_pages};

	for (size_t server_idx = 0; server_idx < preprocessed_servers.size(); ++server_idx)
	{
		t_sserver server;

		std::vector<std::string>& current_server = preprocessed_servers[server_idx];
		
		for (size_t i = 0; i < current_server.size(); ++i)
		{
			std::string& line = current_server[i];
			if (i <= init_functions.size())
			{
				init_functions[i](line, server);
				continue ;
			}
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
