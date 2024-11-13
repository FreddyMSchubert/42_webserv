#include "Config.hpp"

// Expects the string between the {} of a server block
Config::Config(std::string data)
{
	// 1. split into a vector of strings, seperated by ';' or '}'
	std::vector<std::string> lines;
	std::regex delimiter("[;}]");
	std::sregex_token_iterator iter(data.begin(), data.end(), delimiter, -1);
	std::sregex_token_iterator end;
	for (; iter != end; ++iter)
	{
		std::string token = *iter;
		token = std::regex_replace(token, std::regex("^\\s+|\\s+$"), "");
		token = std::regex_replace(token, std::regex("\\s+"), " ");

		if (!token.empty())
			lines.push_back(token);
	}

	// 2. parse each line, based on starting keyword
	std::array<std::string, 7> keywords = {"listen", "server_name", "root", "index", "client_max_body_size", "error_page", "error_page"};
	std::array<void(std::string), 7> parsers = {parseListen, parseServerName, parseRoot, parseIndex, parseClientMaxBodySize, parseErrorPage, parseErrorPage};
	for (std::string &line : lines)
	{
		std::string keyword = line.substr(0, line.find(' '));
		try
		{
			for (int i = 0; i < 7; i++)
			{
				if (keyword == keywords[i])
				{
					parsers[i](line);
					break;
				}
			}
		}
		catch (std::exception &e)
		{
			Logger::Log(LogLevel::ERROR, "Line \"" + line + "\": " + e.what());
		}
	}
}

/* ------------------------ */
/* ----- LINE PARSERS ----- */
/* ------------------------ */

void Config::parseListen(std::string line)
{
	std::regex listen_regex(R"(listen\s+((\d{1,3}(?:\.\d{1,3}){3}):)?(\d+);)");
	std::smatch match;
	if (std::regex_match(line, match, listen_regex))
	{
		if (match[2].matched)
			_host = match[2];
		else
			_host = "0.0.0.0";
		_port = std::stoi(match[3]);
	}
	else
	{
		throw std::invalid_argument("Invalid listen directive");
	}
}

void Config::parseServerName(std::string line)
{
	std::regex server_name_regex(R"(server_name\s+(.+);)");
	std::smatch match;
	if (std::regex_match(line, match, server_name_regex))
	{
		std::string names_str = match[1];
		std::istringstream iss(names_str);
		std::string name;
		while (iss >> name)
			_server_names.push_back(name);
	}
	else
	{
		throw std::invalid_argument("Invalid server_name directive");
	}
}

void Config::parseRoot(std::string line)
{
	std::regex root_regex(R"(root\s+([^;]+);)");
	std::smatch match;
	if (std::regex_match(line, match, root_regex))
		_root_dir = Path::verifyPath(match[1]);
	else
		throw std::invalid_argument("Invalid root directive");
}

void Config::parseIndex(std::string line)
{
	std::regex index_regex(R"(server_name\s+(.+);)");
	std::smatch match;
	if (std::regex_match(line, match, index_regex))
	{
		std::string names_str = match[1];
		std::istringstream iss(names_str);
		std::vector<std::string> names;
		std::string name;
		while (iss >> name)
			names.push_back(name);
		for (int i = 0; i < names.size(); i++)
		{
			try
			{
				_index_file = FilePath(names[i], Path::Type::URL, this);
			}
			catch (std::exception &e)
			{
				// only throw if last index file
				if (i == names.size() - 1)
					throw e;
			}
		}
	}
	else
	{
		throw std::invalid_argument("Invalid server_name directive");
	}
}

void Config::parseClientMaxBodySize(std::string line)
{
	std::regex size_regex(R"(client_max_body_size\s+(\d+)\s*([KMG]B);)", std::regex::icase);
	std::smatch match;
	if (std::regex_match(line, match, size_regex))
	{
		unsigned int size = std::stoi(match[1]);
		std::string unit = match[2];

		if (unit == "B" || unit == "b")
			_client_max_body_size = size;
		else if (unit == "KB" || unit == "kb")
			_client_max_body_size = static_cast<unsigned int>(size) * 1024;
		else if (unit == "MB" || unit == "mb")
			_client_max_body_size = static_cast<unsigned int>(size) * 1024 * 1024;
		else if (unit == "GB" || unit == "gb")
			_client_max_body_size = static_cast<unsigned int>(size) * 1024 * 1024 * 1024;
		else
			throw std::invalid_argument("Unsupported unit in client_max_body_size");
	}
	else
	{
		throw std::invalid_argument("Invalid client_max_body_size directive");
	}
}

void Config::parseErrorPage(std::string line)
{
	std::regex error_page_regex(R"(error_page\s+((?:\d{3}\s+)+)([^;]+);)");
	std::smatch match;
	if (std::regex_match(line, match, error_page_regex))
	{
		std::string error_codes_str = match[1];
		std::string error_path = match[2];
		FilePath path(error_path, Path::Type::URL, this);

		std::istringstream iss(error_codes_str);
		std::string code_str;
		while (iss >> code_str)
		{
			int error_code = std::stoi(code_str);
			_error_pages[error_code] = path;
		}
	}
	else
	{
		throw std::invalid_argument("Invalid error_page directive");
	}
}

void Config::parseLocation(std::string line)
{
	Logger::Log(LogLevel::INFO, "Parsing location: " + line);
	// this will tkae some time. not yet worth it. for now, lets just get this rolling
	// heres some default values though
	t_location location;
	location.path = Path("/", Path::Type::URL, this);
	location.root_dir = Path("/www/clicker/", Path::Type::FILESYSTEM, this);
	location.allowed_methods.emplace(Method::GET, true);
	location.allowed_methods.emplace(Method::POST, true);
	location.allowed_methods.emplace(Method::DELETE, true);
	location.directory_listing = true;
}
