#include "Config.hpp"

// Expects the string between the {} of a server block
Config::Config(std::string data)
{
	std::cout << data
			  << std::endl;


	// 1. split into a vector of strings, seperated by ';' or '}'
	std::vector<std::string> lines;
	std::string token;
	int brace_count = 0;
	for (size_t i = 0; i < data.size(); ++i)
	{
		char c = data[i];
		token += c;

		if (c == '{')
		{
			brace_count++;
		}
		else if (c == '}')
		{
			brace_count--;
			if (brace_count == 0)
			{
				token = std::regex_replace(token, std::regex("^\\s+|\\s+$"), "");
				token = std::regex_replace(token, std::regex("\\s+"), " ");
				if (!token.empty())
					lines.push_back(token);
				token.clear();
			}
		}
		else if (c == ';' && brace_count == 0)
		{
			token = std::regex_replace(token, std::regex("^\\s+|\\s+$"), "");
			token = std::regex_replace(token, std::regex("\\s+"), " ");
			if (!token.empty())
				lines.push_back(token);
			token.clear();
		}
	}

	// 2. parse each line, based on starting keyword
	std::array<std::string, 7> keywords = {"listen", "server_name", "root", "index", "client_max_body_size", "error_page", "location"};
	std::array<void (Config::*)(const std::string&), 7> parsers = {&Config::parseListen, &Config::parseServerName, &Config::parseRoot, &Config::parseIndex, &Config::parseClientMaxBodySize, &Config::parseErrorPage, &Config::parseLocation};
	for (std::string &line : lines)
	{
		std::string keyword = line.substr(0, line.find(' '));
		bool foundMatch = false;
		for (int i = 0; i < 7; i++)
		{
			if (keyword == keywords[i])
			{
				Logger::Log(LogLevel::INFO, "Parsing " + keyword + " line: \"" + line + "\"");
				(this->*parsers[i])(line);
				foundMatch = true;
				break;
			}
		}
		if (!foundMatch)
			throw std::invalid_argument("Invalid directive: \"" + keyword + "\"");
	}
}

/* ------------------------ */
/* ----- LINE PARSERS ----- */
/* ------------------------ */

void Config::parseListen(const std::string & line)
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

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Host: " + _host + ", Port: " + std::to_string(_port));
	#endif
}

void Config::parseServerName(const std::string & line)
{
	std::regex server_name_regex(R"(server_name\s+(.+);)");
	std::smatch match;
	std::string names_str;
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

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Server names: " + names_str);
	#endif
}

void Config::parseRoot(const std::string & line)
{
	std::regex root_regex(R"(root\s+([^;]+);)");
	std::smatch match;
	if (std::regex_match(line, match, root_regex))
		_root_dir = Path::verifyPath(match[1]);
	else
		throw std::invalid_argument("Invalid root directive");

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Root directory: " + _root_dir);
	#endif
}

void Config::parseIndex(const std::string & line)
{
	std::regex index_regex(R"(index\s+(.+);)");
	std::smatch match;
	if (std::regex_match(line, match, index_regex))
	{
		std::string names_str = match[1];
		std::istringstream iss(names_str);
		std::vector<std::string> names;
		std::string name;
		while (iss >> name)
			names.push_back(name);
		for (size_t i = 0; i < names.size(); i++)
		{
			try
			{
				_index_file = FilePath(names[i], Path::Type::URL, *this);
			}
			catch (std::exception &e)
			{
				// only throw if last index file
				Logger::Log(LogLevel::WARNING, "Index file: " + names[i] + " not found: " + e.what());
				if (i == names.size() - 1)
					throw std::runtime_error("No valid index file found.");
			}
		}
	}
	else
	{
		throw std::invalid_argument("Invalid server_name directive");
	}

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Index file: " + _index_file->asUrl());
	#endif
}

void Config::parseClientMaxBodySize(const std::string & line)
{
	std::regex size_regex(R"(client_max_body_size\s+(\d+)\s*([KMG]?B);)", std::regex::icase);
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

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Client max body size: " + std::to_string(_client_max_body_size));
	#endif
}

void Config::parseErrorPage(const std::string & line)
{
	std::regex error_page_regex(R"(error_page\s+((?:\d{3}\s+)+)([^;]+);)");
	std::smatch match;
	if (std::regex_match(line, match, error_page_regex))
	{
		std::string error_codes_str = match[1];
		std::string error_path = match[2];
		FilePath path(error_path, Path::Type::URL, *this);

		std::istringstream iss(error_codes_str);
		std::string code_str;
		while (iss >> code_str)
		{
			int error_code = std::stoi(code_str);
			_error_pages.emplace(error_code, path);
		}
	}
	else
	{
		throw std::invalid_argument("Invalid error_page directive");
	}

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Error pages:");
		for (const auto &page : _error_pages)
			Logger::Log(LogLevel::INFO, "Error " + std::to_string(page.first) + ": " + page.second.asUrl());
	#endif
}

void Config::parseLocation(const std::string & line)
{
	Logger::Log(LogLevel::INFO, "Parsing location: " + line);

	Path locPath = Path("/www/clicker/", Path::Type::FILESYSTEM, *this);
	Path locRoot = Path("/", Path::Type::URL, *this);
	std::unordered_map<Method, bool> methods;
	methods.emplace(Method::GET, true);
	methods.emplace(Method::POST, true);
	methods.emplace(Method::DELETE, true);
	bool directory_listing = true;
	Path locUpload = Path("/www/clicker/uploads/", Path::Type::FILESYSTEM, *this);
	t_location loc = {locPath, locRoot, methods, directory_listing, {}, {}, locUpload};
	_locations.push_back(loc);

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Locations: ");
		for (const t_location &loc : _locations)
		{
			Logger::Log(LogLevel::INFO, "Path: " + (std::holds_alternative<Path>(loc.path) ? std::get<Path>(loc.path).asUrl() : std::get<FilePath>(loc.path).asUrl()));
			Logger::Log(LogLevel::INFO, "Root: " + loc.root_dir.asUrl());
			Logger::Log(LogLevel::INFO, "Upload: " + loc.upload_dir.asUrl());
			Logger::Log(LogLevel::INFO, "Methods: ");
			for (const auto &method : loc.allowed_methods)
				Logger::Log(LogLevel::INFO, "  " + methodToString(method.first) + ": " + std::to_string(method.second));
			Logger::Log(LogLevel::INFO, "Directory listing: " + std::to_string(loc.directory_listing));
			Logger::Log(LogLevel::INFO, "CGI extensions: ");
			for (const std::string &ext : loc.cgi_extensions)
				Logger::Log(LogLevel::INFO, "  " + ext);
			Logger::Log(LogLevel::INFO, "Redirections: ");
			for (const auto &redir : loc.redirections)
				Logger::Log(LogLevel::INFO, "  " + std::to_string(redir.first) + ": " + redir.second.asUrl());
		}
	#endif
}

/* ----------------- */
/* ----- Utils ----- */
/* ----------------- */

t_location Config::getRootLocation() const
{
	for (const t_location &loc : _locations)
		if (loc.root_dir.asUrl() == "/")
			return loc;
	throw std::runtime_error("Root location not present in config " + _root_dir);
}
