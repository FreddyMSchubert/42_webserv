#include "../../include/Packets/Packet.hpp"
#include <sstream>

Packet::Packet(const std::string &rawPacket)
{
	std::istringstream iss(rawPacket);
	std::string line;
	std::string headers;
	std::string body;

	std::getline(iss, line);
	ParseRequestLine(line);

	while (std::getline(iss, line))
	{
		if (line.find(":") == std::string::npos)
			break;
		headers += line + "\n";
	}
	ParseHeaders(headers);

	while (std::getline(iss, line))
		body += line + "\n";
	ParseBody(body);
}

Packet::Packet(Method method, const std::string path, const std::string version, const std::map<std::string, std::string> headers, const std::string body)
{
	_method = method;
	_path = path;
	_version = version;
	_headers = headers;
	_body = body;
}

void Packet::ParseRequestLine(std::string &line)
{
	std::string::size_type separator = line.find(' ');
	if (separator == std::string::npos)
		throw std::runtime_error("Invalid packet request line");
	std::string method = line.substr(0, separator);
	std::istringstream stream(line.substr(separator, line.size() - 1));

	if (method == "GET")
		_method = Method::GET;
	else if (method == "POST")
		_method = Method::POST;
	else if (method == "DELETE")
		_method = Method::DELETE;
	else
		_method = Method::UNKNOWN;

	stream  >> _path >> _version;
}

void Packet::ParseHeaders(std::string &headers)
{
	std::istringstream iss(headers);
	std::string line;
	while (std::getline(iss, line))
	{
		if (!line.empty() && line.back() == '\r') // windows artifacts
			line.pop_back();

		if (line.empty())
			continue;

		std::string::size_type separator = line.find(':');
		if (separator == std::string::npos)
			throw std::runtime_error("Invalid packet header");

		std::string key(line.substr(0, separator));
		std::string value(line.substr(separator + 1));

		// trim whitespace
		key.erase(0, key.find_first_not_of(" \t"));
		key.erase(key.find_last_not_of(" \t") + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t") + 1);

		_headers[key] = value;
	}
}

void Packet::ParseBody(std::string &body)
{
	// Chunked Transfer Encoding
	if (_headers.find("Transfer-Encoding") != _headers.end() && _headers["Transfer-Encoding"] == "chunked")
	{
		std::istringstream iss(body);
		std::string line;
		std::string chunkSize;
		std::string chunk;
		while (std::getline(iss, line))
		{
			if (line.empty())
				continue;
			if (line.back() == '\r') // windows artifacts
				line.pop_back();

			if (chunkSize.empty())
			{
				chunkSize = line;
				continue;
			}
			if (line.size() == std::stoul(chunkSize, nullptr, 16))
			{
				_body += chunk;
				chunkSize.clear();
				chunk.clear();
				continue;
			}
			chunk += line;
		}
	}
	// Content-Length header
	else if (_headers.find("Content-Length") != _headers.end())
	{
		size_t contentLength = std::stoi(_headers["Content-Length"]);
		if (body.size() < contentLength)
			throw std::runtime_error("Invalid body size");
		_body = body.substr(0, contentLength);
	}
	// No specified body encoding
	else
	{
		_body = body;
		// throw std::runtime_error("Unsupported transfer encoding");
	}
}

Method Packet::getMethod() { return _method; }
std::string Packet::getPath() { return _path; }
std::string Packet::getVersion() { return _version; }
std::map<std::string, std::string> Packet::getHeaders() { return _headers; }
std::string Packet::getBody() { return _body; }

void Packet::logData()
{
	std::cout << "Method: ";
	switch (_method)
	{
		case Method::GET: std::cout << "GET" << std::endl; break;
		case Method::POST: std::cout << "POST" << std::endl; break;
		case Method::DELETE: std::cout << "DELETE" << std::endl; break;
		default: std::cout << "UNKNOWN" << std::endl; break;
	}
	std::cout << "Path: " << _path << std::endl;
	std::cout << "Version: " << _version << std::endl;
	std::cout << "Headers: " << std::endl;
	for (auto &header : _headers)
		std::cout << header.first << ": " << header.second << std::endl;
	std::cout << "Body: " << _body << std::endl;
	std::cout << "---" << std::endl;
}

Packet::Packet(Packet const &src)
{
	_headers.clear();

	_method = src._method;
	_path = src._path;
	_version = src._version;
	_headers = src._headers;
	_body = src._body;
}

Packet &Packet::operator=(Packet const &src)
{
	if (&src == this) return *this;

	_headers.clear();

	_method = src._method;
	_path = src._path;
	_version = src._version;
	_headers = src._headers;
	_body = src._body;
	return *this;
}

