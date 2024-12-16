#include "Config.hpp"

Config::Config()
{
}

// Expects the string between the {} of a server block
Config::Config(std::string data)
{
	// 1. split into a vector of strings, seperated by ';' or '}'
	std::vector<std::string> lines;
	extractConfigFromBrackets(lines, static_cast<const std::string&>(data));

	// 2. parse each line, based on starting keyword
	std::array<std::string, CONFIG_KEYWORD_COUNT> keywords = {"listen", "server_name", "root", "index", "client_max_body_size", "error_page", "location", "client_timeout"};
	std::array<void (Config::*)(const std::string&), CONFIG_KEYWORD_COUNT> parsers = {&Config::parseListen, &Config::parseServerName, &Config::parseRoot, &Config::parseIndex, &Config::parsemaxPackageSize, &Config::parseErrorPage, &Config::parseLocation, &Config::parseClientTimeout};
	for (std::string &line : lines)
	{
		std::string keyword = line.substr(0, line.find(' '));
		bool foundMatch = false;
		for (int i = 0; i < CONFIG_KEYWORD_COUNT; i++)
		{
			if (keyword == keywords[i])
			{
				#if LOG_CONFIG_PARSING
					Logger::Log(LogLevel::INFO, "Parsing " + keyword + " line: \"" + line + "\"");
				#endif
				(this->*parsers[i])(line);
				foundMatch = true;
				break;
			}
		}
		if (!foundMatch)
			throw std::invalid_argument("Invalid directive: \"" + keyword + "\"");
	}
	if (_host.empty()) throw std::invalid_argument("No listen directive found");
	if (_port == 0) throw std::invalid_argument("No listen directive found");
	if (_root_dir.empty()) throw std::invalid_argument("No root directive found");
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

void Config::parsemaxPackageSize(const std::string & line)
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
			Logger::Log(LogLevel::WARNING, "Invalid unit in client_max_body_size directive. Using default value (1MB).");
	}
	else
	{
		Logger::Log(LogLevel::WARNING, "Invalid unit in client_max_body_size directive. Using default value (1MB).");
	}

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Client max body size: " + std::to_string(_client_max_body_size));
	#endif
}

void Config::parseErrorPage(const std::string & line)
{
	std::regex error_page_regex(R"(error_page\s+((?:[1-5]\d{2}\s+)+)([^;]+);)");
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
			for (auto error : _error_pages)
				if (error.first == error_code)
					throw std::invalid_argument("Invalid error_page directive");
			_error_pages.emplace(error_code, path);
		}
	}
	else
		throw std::invalid_argument("Invalid error_page directive");

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Error pages:");
		for (const auto &page : _error_pages)
			Logger::Log(LogLevel::INFO, "Error " + std::to_string(page.first) + ": " + page.second.asUrl());
	#endif
}

void Config::parseClientTimeout(const std::string & line)
{
	std::regex timeout_regex(R"(^\s*client_timeout\s+(\d+)\s*(ms|MS|s|S)\s*;?\s*$)");
	std::smatch match;
	if (std::regex_match(line, match, timeout_regex))
	{
		int value = std::stoi(match[1]);
		std::string unit = match[2];

		if (unit == "ms" || unit == "MS")
		{
			_client_timeout = value;
		}
		else if (unit == "s" || unit == "S")
		{
			_client_timeout = value * 1000;
		}
		else
		{
			throw std::invalid_argument("Unsupported unit in client_timeout directive");
		}
	}
	else
		Logger::Log(LogLevel::WARNING, "Invalid client_timeout directive. Using default value (30s).");

	#if LOG_CONFIG_PARSING
		Logger::Log(LogLevel::INFO, "Client timeout: " + std::to_string(_client_timeout));
	#endif
}

/* --------------------------------- */
/* ----- LOCATION LINE PARSERS ----- */
/* --------------------------------- */

void Config::parseLocation(const std::string& line)
{
	std::vector<std::string> configLines;
	std::string location_path;
	size_t pos = 0;

	// Step 1: Extract location path
	while (pos < line.size() && std::isspace(line[pos])) ++pos;
	if (line.substr(pos, 8) == "location")
	{
		pos += 8;
		while (pos < line.size() && std::isspace(line[pos])) ++pos;
		while (pos < line.size() && !std::isspace(line[pos]) && line[pos] != '{')
			location_path += line[pos++];
		while (pos < line.size() && std::isspace(line[pos])) ++pos;
		++pos; // Skip '{' 

		// Step 2: Extract config inside the braces
		extractConfigFromBrackets(configLines, line.substr(pos));

		// Step 3: Initialize location with default values
		t_location loc = {
			Path(location_path, Path::Type::URL, *this),
			"",
			{false, false, false},
			false,
			{},
			{},
			Path(_root_dir + "/", Path::Type::FILESYSTEM, *this)
		};

		// Step 4: Define keywords and corresponding parser functions
		std::array<std::string, 6> keywords = {"allowed_methods", "root", "autoindex", "upload_dir", "cgi_extensions", "return"};
		std::array<void (Config::*)(const std::string&, t_location&), 6> parsers = {&Config::parseLocationAllowedMethods, &Config::parseLocationRoot,  &Config::parseLocationAutoindex, &Config::parseLocationUploadDir, &Config::parseLocationCgiExtensions, &Config::parseLocationRedirections};
		for (const auto& configLine : configLines)
		{
			bool foundMatch = false;
			for (size_t i = 0; i < keywords.size(); ++i)
			{
				if (configLine.find(keywords[i]) != std::string::npos)
				{
					#if LOG_CONFIG_PARSING
						Logger::Log(LogLevel::INFO, "Parsing " + keywords[i] + " line: \"" + configLine + "\"");
					#endif
					(this->*parsers[i])(configLine, loc);
					foundMatch = true;
					break;
				}
			}
			if (!foundMatch)
				throw std::invalid_argument("Invalid directive: \"" + configLine + "\"");
		}

		_locations.push_back(loc);
	}
	else
		throw std::invalid_argument("Location could not be found!");

	#if LOG_CONFIG_PARSING
		for (const t_location &loc : _locations)
			std::cout << loc << std::endl;
	#endif
}

// root /www/clicker/custom_data/2024/assets;
// TODO: Make this handle files, not just dirs
void Config::parseLocationRoot(const std::string & line, t_location & loc)
{
	std::regex root_regex(R"(^\s*root\s+\/[a-zA-Z0-9\/.]*\s*;\s*$)");
	std::smatch match;
	if (std::regex_match(line, match, root_regex))
		loc.root_dir = Path::verifyPath(match[1]);
	else
		throw std::invalid_argument("Invalid location root directive: \"" + line + "\"");
}

void Config::parseLocationAllowedMethods(const std::string & line, t_location & loc)
{
	std::regex allowed_methods_regex(R"(\s*allowed_methods\s+([A-Z\s]+)\s*;\s*)");
	std::smatch match;
	if (std::regex_match(line, match, allowed_methods_regex))
	{
		std::string methods_str = match[1];
		std::istringstream iss(methods_str);
		std::string method;
		while (iss >> method)
		{
			if (method == "GET")
				loc.allowed_methods[0] = true;
			else if (method == "POST")
				loc.allowed_methods[1] = true;
			else if (method == "DELETE")
				loc.allowed_methods[2] = true;
			else
				throw std::invalid_argument("Invalid method in allowed_methods directive: \"" + method + "\"");
		}
	}
	else
		throw std::invalid_argument("Invalid location allowed_methods directive: \"" + line + "\"");
}

void Config::parseLocationAutoindex(const std::string & line, t_location & loc)
{
	std::regex autoindex_regex(R"(^\s*autoindex\s*(on|off)\s*;\s*$)");
	std::smatch match;
	if (std::regex_match(line, match, autoindex_regex))
	{
		if (line.find("on"))
			loc.directory_listing = true;
		else if (line.find("off"))
			loc.directory_listing = false;
	}
	else
		Logger::Log(LogLevel::WARNING, "Invalid location autoindex directive. Using default value (off).");
}

void Config::parseLocationCgiExtensions(const std::string & line, t_location & loc)
{
	std::regex cgi_extensions_regex(R"(\s*cgi_extensions\s+(\.[a-zA-Z0-9]+(?:\s+\.[a-zA-Z0-9]+)*)\s*;\s*)");
	std::smatch match;
	if (std::regex_match(line, match, cgi_extensions_regex))
	{
		std::string extensions_str = match[1].str();
		std::istringstream iss(extensions_str);
		std::string extension;
		while (iss >> extension)
			loc.cgi_extensions.push_back(extension);
	}
	else
		throw std::invalid_argument("Invalid location CGI extensions directive: \"" + line + "\"");
}

void Config::parseLocationRedirections(const std::string &line, t_location &loc)
{
	std::regex redirection_regex(R"(\s*return\s+(\d{3})\s+([^\s;]+)\s*;\s*)");
	std::smatch match;
	if (std::regex_match(line, match, redirection_regex))
	{
		int status_code = std::stoi(match[1].str());
		std::string redirect_path = match[2].str();
		redirect_path = Path::combinePaths(loc.getPathAsPath().asFilePath(), redirect_path);
		loc.redirections.emplace(status_code, FilePath(redirect_path, Path::Type::FILESYSTEM, *this));
	}
	else
		throw std::invalid_argument("Invalid location redirect directive: \"" + line + "\"");
}

void Config::parseLocationUploadDir(const std::string &line, t_location &loc)
{
	std::regex upload_dir_regex(R"(\s*upload_dir\s+([^\s;]+)\s*;\s*)");
	std::smatch match;
	if (std::regex_match(line, match, upload_dir_regex))
	{
		std::string upload_path = match[1].str();
		upload_path = Path::combinePaths(loc.getPathAsPath().asFilePath(), upload_path);
		loc.upload_dir = Path(upload_path, Path::Type::FILESYSTEM, *this);
	}
	else
		throw std::invalid_argument("Invalid location upload_dir directive: \"" + line + "\"");
}

/* ----------------- */
/* ----- UTILS ----- */
/* ----------------- */

void	Config::extractConfigFromBrackets(std::vector<std::string> &lines, const std::string &data)
{
	std::string token;
	int brace_count = 0;

	for (size_t i = 0; i < data.size(); ++i)
	{
		char c = data[i];
		token += c;

		if (c == '{')
			brace_count++;
		else if (c == '}' || c == ';')
		{
			if (c == '}')
				brace_count--;
			if (brace_count == 0)
			{
				size_t comment_pos = token.find('#');
				if (comment_pos != std::string::npos)
					token = token.substr(0, comment_pos);

				token = std::regex_replace(token, std::regex("^\\s+|\\s+$"), "");
				token = std::regex_replace(token, std::regex("\\s+"), " ");
				if (!token.empty())
					lines.push_back(token);
				token.clear();
			}
		}
	}
}

t_location Config::getRootLocation() const
{
	for (const t_location &loc : getLocations())
	{
		if (std::holds_alternative<Path>(loc.path) && std::get<Path>(loc.path).asUrl() == "/")
			return loc;
		if (std::holds_alternative<FilePath>(loc.path) && std::get<FilePath>(loc.path).asUrl() == "/")
			return loc;
	}
	throw std::runtime_error("Root location not present in config " + getRootDir());
}

std::ostream &operator<<(std::ostream &os, const t_location &loc)
{
	os << "Path: " + (std::holds_alternative<Path>(loc.path) ? std::get<Path>(loc.path).asUrl() : std::get<FilePath>(loc.path).asUrl()) << " ";
	os << "Root: " + loc.root_dir << " ";
	os << "Upload: " + loc.upload_dir.asUrl() << " ";
	os << "Methods:";
	for (int i = 0; i < 3; i++)
		os << " " + methodToString(static_cast<Method>(i)) + ": " + (loc.allowed_methods.at(i) ? "true" : "false");
	os << std::string(" Directory listing: ") + (loc.directory_listing ? "true" : "false") << " ";
	os << "CGI extensions: ";
	for (const std::string &ext : loc.cgi_extensions)
		os << "  " + ext;
	os << " Redirections: ";
	for (const auto &redir : loc.redirections)
		os << "  " + std::to_string(redir.first) + ": " + redir.second.asUrl();
	
	return os;
}
