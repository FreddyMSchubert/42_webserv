#include "../include/Config.hpp"

std::vector<std::string> read_file(const std::string& path_to_conf)
{
    std::vector<std::string> result_str;
    std::ifstream conf(path_to_conf);
    std::string line;

    if (conf.is_open() == false)
	{
        std::cerr << "Config file could not be opened!" << std::endl;
		exit(1); //can we do that here? or do we have data leaks somewhere then?
    }

    while (std::getline(conf, line))
        result_str.push_back(line);

    conf.close();
    return (result_str);
}

bool	is_empty_or_whitespace(const std::string& str)
{
	return (str.find_first_not_of(" \t\n\r") == std::string::npos);
}

bool	start_of_a_server(const std::string& str)
{
	std::regex pattern(R"(^\s*server\s*\{\s*$)");
	return (std::regex_match(str, pattern));
}

int		end_of_a_server(std::vector<std::string> conf, int i)
{
	return (69420);
}

void	split_conf_in_server(t_sserver_configs server, std::vector<std::string> conf)
{
	int	start_of_server = -1;
	int	end_of_server = -1;

	for (int i = 0; i < conf.size(); ++i)
	{
		if (is_empty_or_whitespace(conf[i]) == true)
			continue ;
		if (start_of_a_server(conf[i]) == true)
		{
			if (start_of_server == -1)
				start_of_server = i;
			end_of_server = end_of_a_server(conf, i);
		}
		if (start_of_server != -1 && end_of_server != -1)
		{
			t_sserver tmp_server;

			while (start_of_server <= end_of_server)
			{
				tmp_server.each_server.push_back(conf[start_of_server]);
				start_of_server++;
			}

			server.server_list.push_back(tmp_server);
			start_of_server = -1;
			end_of_server = -1;
		}
	}
}

t_sserver_configs	get_config(bool use_own_conf, char *argv[])
{
	t_sserver_configs			tmp_serv_conf;

	if (use_own_conf == true)
		split_conf_in_server(tmp_serv_conf, read_file("../config/default.conf"));
	if (use_own_conf == false)
		split_conf_in_server(tmp_serv_conf, read_file(argv[1]));
	return (tmp_serv_conf);
}
