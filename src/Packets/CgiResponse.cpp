#include "Response.hpp"

void Response::handleCgiResponse(Request &req, Config &config)
{
	// 1. Determine Script Path
	FilePath fpath(req.getPath(), Path::Type::URL, config);
	std::string extension = fpath.getFileExtension();
	std::string exec_cmd;

	std::vector<t_location> locations = get_locations(config, req.getPath());
	for (t_location loc : locations)
	{
		std::vector<std::pair<std::string, std::string>> cgi_extensions = loc.cgi_extensions;
		for (std::pair<std::string, std::string> ext : cgi_extensions)
		{
			if (ext.first == extension)
			{
				exec_cmd = ext.second;
				break;
			}
		}
	}

	// 2. Apply / insert script
	size_t scriptPlaceholder = exec_cmd.find("[SCRIPT]");
	if (scriptPlaceholder != std::string::npos)
		exec_cmd.replace(scriptPlaceholder, 8, Path::combinePaths(std::filesystem::current_path(), Path::combinePaths(config.getRootDir(), req.getPath())));
	else
		throw std::runtime_error("No script placeholder found in cgi command");

	// 3. split up & convert command
	std::vector<std::string> cmdParts;
	std::istringstream iss(exec_cmd);
	std::string part;
	while (std::getline(iss, part, ' '))
		cmdParts.push_back(part);
	if (cmdParts.empty())
		throw std::runtime_error("No command found in cgi command");

	std::vector<char *> argv;
	argv.reserve(cmdParts.size() + 1);
	for (std::string &part : cmdParts)
		argv.push_back(const_cast<char *>(part.c_str()));
	argv.push_back(nullptr);

	// 4. Set up environment variables
	/*
		Key	Value
		DOCUMENT_ROOT	The root directory of your server
		HTTP_COOKIE	The visitor's cookie, if one is set
		HTTP_HOST	The hostname of the page being attempted
		HTTP_REFERER	The URL of the page that called your program
		HTTP_USER_AGENT	The browser type of the visitor
		HTTPS	"on" if the program is being called through a secure server
		PATH	The system path your server is running under
		QUERY_STRING	The query string (see GET, below)
		REMOTE_ADDR	The IP address of the visitor
		REMOTE_HOST	The hostname of the visitor (if your server has reverse-name-lookups on; otherwise this is the IP address again)
		REMOTE_PORT	The port the visitor is connected to on the web server
		REMOTE_USER	The visitor's username (for .htaccess-protected pages)
		REQUEST_METHOD	GET or POST
		REQUEST_URI	The interpreted pathname of the requested document or CGI (relative to the document root)
		SCRIPT_FILENAME	The full pathname of the current CGI
		SCRIPT_NAME	The interpreted pathname of the current CGI (relative to the document root)
		SERVER_ADMIN	The email address for your server's webmaster
		SERVER_NAME	Your server's fully qualified domain name (e.g. www.cgi101.com)
		SERVER_PORT	The port number your server is listening on
		SERVER_SOFTWARE	The server software you're using (e.g. Apache 1.3)
	*/

	std::map<std::string, std::string> env;
	env["DOCUMENT_ROOT"] = ".";
	env["HTTP_HOST"] = config.getHost();
	env["HTTP_USER_AGENT"] = req.getHeader("User-Agent");
	env["HTTPS"] = req.getVersion().find("HTTPS") != std::string::npos ? "on" : "off";
	env["PATH"] = std::filesystem::current_path();
	env["HTTP_COOKIE"] = req.getHeader("Cookie");
	// probably add more here
	// -> https://www.cgi101.com/book/ch3/text.html

	std::vector<std::string> envStrings;
	for (auto &pair : env)
		envStrings.push_back(pair.first + "=" + pair.second);
	std::vector<char *> envp;
	for (std::string &envString : envStrings)
		envp.push_back(const_cast<char *>(envString.c_str()));
	envp.push_back(nullptr);

	// 5. Fork
	int pipeIn[2];
	int pipeOut[2];

	if (pipe(pipeIn) == -1)
		throw std::runtime_error("Failed to create pipe for cgi input");
	if (pipe(pipeOut) == -1)
		throw std::runtime_error("Failed to create pipe for cgi output");
	
	pid_t pid = fork();
	if (pid < 0)
		throw std::runtime_error("Failed to fork for cgi execution");

	std::cout << "Executing CGI script: " << argv[0] << " with args: ";
	for (size_t i = 0; i < argv.size() - 1; i++)
		std::cout << argv[i] << " ";
	std::cout << std::endl;

	// 6a. Child Process - Execute CGI script
	if (pid == 0)
	{
		if (dup2(pipeIn[0], STDIN_FILENO) == -1)
			throw std::runtime_error("Failed to redirect stdin for cgi script");
		close(pipeIn[0]);
		close(pipeIn[1]);
		if (dup2(pipeOut[1], STDOUT_FILENO) == -1)
			throw std::runtime_error("Failed to redirect stdout for cgi script");
		if (dup2(pipeOut[1], STDERR_FILENO) == -1)
			throw std::runtime_error("Failed to redirect stderr for cgi script");
		close(pipeOut[0]);
		close(pipeOut[1]);

		execve(argv[0], argv.data(), envp.data());

		// If execve returns, an error occurred
		std::cerr << "Failed to execute CGI script: " + std::string(strerror(errno)) << std::endl;
		_exit(1);
	}

	// 6b. Parent Process - Insert input & extract output
	close(pipeIn[0]);
	close(pipeOut[1]);

	const std::string &requestData = req.getBody(); // relevant header info is already in env
	ssize_t totalWritten = 0;
	ssize_t bodyLength = requestData.size();
	while (totalWritten < bodyLength) {
		ssize_t written = write(pipeIn[1], requestData.c_str() + totalWritten, bodyLength - totalWritten);
		if (written <= 0)
			break;
		totalWritten += written;
	}
	close(pipeIn[1]);

	std::string cgiOutput;
	char buffer[4096];
	ssize_t bytesRead;
	while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0) {
		cgiOutput.append(buffer, bytesRead);
	}
	close(pipeOut[0]);

	// 7. Wait for child process to finish
	int status;
	waitpid(pid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		setStatus(Status::InternalServerError);
		setBody("500 Internal Server Error: CGI script execution failed.\n");
		return;
	}
	
	// 8. Parse output
	size_t headerEnd = cgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		headerEnd = cgiOutput.find("\n\n");
	}
	if (headerEnd == std::string::npos) {
		setStatus(Status::InternalServerError);
		setBody("500 Internal Server Error: Invalid CGI script output.\n");
		return;
	}

	std::string headerPart = cgiOutput.substr(0, headerEnd);
	std::string bodyPart = cgiOutput.substr(headerEnd + ((cgiOutput[headerEnd + 1] == '\n') ? 2 : 4));

	std::istringstream headerStream(headerPart);
	std::string headerLine;
	while (std::getline(headerStream, headerLine))
	{
		if (!headerLine.empty() && headerLine.back() == '\r')
			headerLine.pop_back();
		size_t colon = headerLine.find(':');
		if (colon != std::string::npos)
		{
			std::string key = headerLine.substr(0, colon);
			std::string value = headerLine.substr(colon + 1);

			key.erase(0, key.find_first_not_of(" \t"));
			key.erase(key.find_last_not_of(" \t") + 1);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);

			getHeaders()[key] = value;
		}
	}
	setBody(bodyPart);
}
