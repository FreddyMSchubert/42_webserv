#include "Config.hpp"

// Expects the string between the {} of a server block
Config::Config(std::string data)
{
	// 1. split into a vector of strings, seperated by ';' or '}'
	std::vector<std::string> lines;
	extractConfigFromBrackets(lines, static_cast<const std::string&>(data));

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

void Config::parseLocation(const std::string& line)
{
    std::vector<std::string> configLines;
    std::string location_path;
    size_t pos = 0;

    // Step 1: Extract location path
    while (pos < line.size() && std::isspace(line[pos])) ++pos;
    if (line.substr(pos, 8) == "location") {
        pos += 8;
        while (pos < line.size() && std::isspace(line[pos])) ++pos;
        while (pos < line.size() && !std::isspace(line[pos]) && line[pos] != '{')
            location_path += line[pos++];
        while (pos < line.size() && std::isspace(line[pos])) ++pos;
        ++pos; // Skip '{' 
        // Step 2: Extract config inside the braces
        extractConfigFromBrackets(configLines, line.substr(pos));
		// TODO: should we initalize with default values??
        // Step 3: Initialize location with default values
        t_location loc = {
            Path(_root_dir, Path::Type::FILESYSTEM, *this),
            Path(location_path, Path::Type::URL, *this),
            {},
            false,
            {},
            {},
            Path(_root_dir + "/", Path::Type::FILESYSTEM, *this)
        };
        // Step 4: Define keywords and corresponding parser functions
		std::array<std::string, 5> keywords = {"allowed_methods", "autoindex", "upload_dir", "cgi_extensions", "return"};
		std::array<void (Config::*)(const std::string&, t_location&), 5> parsers = {&Config::parseAllowedMethods, &Config::parseAutoindex, &Config::parseUploadDir, &Config::parseCgiExtensions, &Config::parseRedirections};
        // Step 5: Parse each config line and call appropriate parser function
        for (const auto& config : configLines) {
            bool foundMatch = false;
            for (size_t i = 0; i < keywords.size(); ++i) {
                if (config.find(keywords[i]) != std::string::npos) {
                    Logger::Log(LogLevel::INFO, "Parsing " + keywords[i] + " line: \"" + config + "\"");
                    (this->*parsers[i])(config, loc);
                    foundMatch = true;
                    break;
                }
            }
            if (!foundMatch) {
                Logger::Log(LogLevel::ERROR, "Unknown config keyword in location: \"" + config + "\"");
                throw std::invalid_argument("Invalid config keyword: \"" + config + "\"");
            }
        }
        // Step 6: Push populated location to locations vector
        _locations.push_back(loc);
    }
    else
        throw std::invalid_argument("Location could not be found!");
	#if LOG_CONFIG_PARSING
		for (const t_location &loc : _locations)
		{
			Logger::Log(LogLevel::INFO, "Path: " + (std::holds_alternative<Path>(loc.path) ? std::get<Path>(loc.path).asUrl() : std::get<FilePath>(loc.path).asUrl()));
			Logger::Log(LogLevel::INFO, "Root: " + loc.root_dir.asUrl());
			Logger::Log(LogLevel::INFO, "Upload: " + loc.upload_dir.asUrl());
			Logger::Log(LogLevel::INFO, "Methods: ");
			for (const auto &method : loc.allowed_methods)
				Logger::Log(LogLevel::INFO, "  " + methodToString(method.first) + ": " + (method.second ? "true" : "false"));
			Logger::Log(LogLevel::INFO, std::string("Directory listing: ") + (loc.directory_listing ? "true" : "false"));
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

void Config::parseAllowedMethods(const std::string & line, t_location & loc)
{
    std::regex allowed_methods_regex(R"(\s*allowed_methods\s+([A-Z\s]+)\s*;\s*)");
    std::smatch match;
    if (std::regex_match(line, match, allowed_methods_regex))
	{
        std::string methods_str = match[1];
        std::istringstream iss(methods_str);
        std::unordered_map<Method, bool> methods;
        std::string method;
        while (iss >> method)
		{
            if (method == "GET")
                methods[Method::GET] = true;
            else if (method == "POST")
                methods[Method::POST] = true;
            else if (method == "DELETE")
                methods[Method::DELETE] = true;
        }
        loc.allowed_methods = methods;
    }
    else
        throw std::invalid_argument("Invalid allowed_methods line: \"" + line + "\"");
}

void Config::parseAutoindex(const std::string & line, t_location & loc)
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
        throw std::invalid_argument("Invalid autoindex (directory_listing) line: \"" + line + "\"");
}

//FIXME: no fcking clue what we should do with that or how to test that but too dehydrated to be interested in such bs
void Config::parseCgiExtensions(const std::string & line, t_location & loc)
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
        throw std::invalid_argument("Invalid CGI extensions line: \"" + line + "\"");
}

void Config::parseRedirections(const std::string &line, t_location &loc)
{
    std::regex redirection_regex(R"(\s*return\s+(\d{3})\s+([^\s;]+)\s*;\s*)");
    std::smatch match;
    if (std::regex_match(line, match, redirection_regex))
    {
        int status_code = std::stoi(match[1].str());
        std::string redirect_path = match[2].str();
		(void)status_code;
		(void)redirect_path;
		(void)loc;
        // loc.redirections[status_code] = Path(redirect_path, Path::Type::URL, *this); 
    }
    else
        throw std::invalid_argument("Invalid redirect line: \"" + line + "\"");
}

void Config::parseUploadDir(const std::string &line, t_location &loc)
{
    std::regex upload_dir_regex(R"(\s*upload_dir\s+([^\s;]+)\s*;\s*)");
    std::smatch match;
    if (std::regex_match(line, match, upload_dir_regex))
    {
        std::string upload_path = match[1].str();
        loc.upload_dir = Path(upload_path, Path::Type::FILESYSTEM, *this);
    }
    else
        throw std::invalid_argument("Invalid upload_dir line: \"" + line + "\"");
}

void	Config::extractConfigFromBrackets(std::vector<std::string> &lines, const std::string &data)
{
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
}

t_location Config::getRootLocation() const
{
	for (const t_location &loc : _locations)
		if (loc.root_dir.asUrl() == "/")
			return loc;
	throw std::runtime_error("Root location not present in config " + _root_dir);
}
