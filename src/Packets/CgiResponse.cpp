#include "Response.hpp"

void Response::handleCgiResponse(Request &req, Config &config)
{
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

	size_t scriptPlaceholder = exec_cmd.find("[SCRIPT]");
	if (scriptPlaceholder != std::string::npos)
		exec_cmd.replace(scriptPlaceholder, 8, "." + Path::combinePaths(config.getRootDir(), req.getPath()));
	else
		throw std::runtime_error("No script placeholder found in cgi command");

	std::vector<std::string> cmdParts;
	std::istringstream iss(exec_cmd);
	std::string executable;
	std::string part;
	std::getline(iss, executable, ' ');
	while (std::getline(iss, part, ' '))
		cmdParts.push_back(part);
	if (cmdParts.empty())
		throw std::runtime_error("No command found in cgi command");

	std::vector<char *> argv;
	for (std::string &part : cmdParts)
		argv.push_back(const_cast<char *>(part.c_str()));
	argv.push_back(nullptr);

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
	env["HTTP_USER_AGENT"] = req.getHeaders().at("User-Agent");
	env["HTTPS"] = req.getVersion().find("HTTPS") != std::string::npos ? "on" : "off";
	env["PATH"] = std::filesystem::current_path();
	// probably add more here
	// -> https://www.cgi101.com/book/ch3/text.html

	std::vector<std::string> envStrings;
	for (auto &pair : env)
		envStrings.push_back(pair.first + "=" + pair.second);
	std::vector<char *> envp;
	for (std::string &envString : envStrings)
		envp.push_back(const_cast<char *>(envString.c_str()));
	envp.push_back(nullptr);

	int pipeIn[2];
	int pipeOut[2];

	if (pipe(pipeIn) == -1)
		throw std::runtime_error("Failed to create pipe for cgi input");
	if (pipe(pipeOut) == -1)
		throw std::runtime_error("Failed to create pipe for cgi output");
	
	pid_t pid = fork();
	if (pid < 0)
		throw std::runtime_error("Failed to fork for cgi execution");

	std::cout << "Executing CGI script: " << executable << " with args: ";
	for (size_t i = 0; i < argv.size() - 1; i++)
		std::cout << argv[i] << " ";
	std::cout << std::endl;

	if (pid == 0)
	{
		dup2(pipeIn[0], STDIN_FILENO);
		close(pipeIn[0]);
		close(pipeIn[1]);
		
		// Redirect stdout
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeOut[0]);
		close(pipeOut[1]);
		
		// Execute the CGI script
		// log the execve command
		execve(executable.c_str(), argv.data(), envp.data());
		// execve("/bin/ls", nullptr, nullptr);

		// If execve returns, an error occurred
		std::string error = "Failed to execute CGI script: " + std::string(strerror(errno)) + "\n";
		std::cerr << error << std::endl;
		_exit(1);
	}

	close(pipeIn[0]);
	close(pipeOut[1]);

	// Write the request body to the CGI script's stdin
    const std::string &requestBody = req.getBody();
    ssize_t totalWritten = 0;
    ssize_t bodyLength = requestBody.size();
    while (totalWritten < bodyLength) {
        ssize_t written = write(pipeIn[1], requestBody.c_str() + totalWritten, bodyLength - totalWritten);
        if (written <= 0) {
            break; // Error or EOF
        }
        totalWritten += written;
    }
    close(pipeIn[1]); // Finished writing
    
    // Read the CGI script's output from stdoutPipe
    std::string cgiOutput;
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0) {
        cgiOutput.append(buffer, bytesRead);
    }
    close(pipeOut[0]);
    
    // Wait for the child process to finish
    int status;
    waitpid(pid, &status, 0);
    
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        setStatus(Status::InternalServerError);
        setBody("500 Internal Server Error: CGI script execution failed.\n");
        return;
    }
    
    // Step 5: Parse CGI output (headers and body)
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        headerEnd = cgiOutput.find("\n\n");
    }
    if (headerEnd == std::string::npos) {
        // No headers found
        setStatus(Status::InternalServerError);
        setBody("500 Internal Server Error: Invalid CGI script output.\n");
        return;
    }
    
    std::string headerPart = cgiOutput.substr(0, headerEnd);
    std::string bodyPart = cgiOutput.substr(headerEnd + ((cgiOutput[headerEnd + 1] == '\n') ? 2 : 4));
    
    // Parse headers
    std::istringstream headerStream(headerPart);
    std::string headerLine;
    while (std::getline(headerStream, headerLine)) {
        // Remove any trailing \r
        if (!headerLine.empty() && headerLine.back() == '\r') {
            headerLine.pop_back();
        }
        size_t colon = headerLine.find(':');
        if (colon != std::string::npos) {
            std::string key = headerLine.substr(0, colon);
            std::string value = headerLine.substr(colon + 1);
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
			getHeaders()[key] = value;
        }
    }
    
    // Set the response body
    setBody(bodyPart);
    
    // Optionally set the status code if provided by CGI
	// if (getHeaders().find("Status") != getHeaders().end()) {
	// 	int statusCode = std::stoi(getHeaders()["Status"]);
	// 	setStatus(statusCode);
	// } else {
	// 	setStatus(Status::OK);
	// }
}
