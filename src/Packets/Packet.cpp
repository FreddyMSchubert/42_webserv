#include "../../include/Packet.hpp"
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
		if (line == "\r")
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
	std::string method = line.substr(0, line.find(' '));
	std::istringstream stream(line.substr(line.find(' '), line.size() - 1));

	if (method == "GET")
		_method = Method::GET;
	else if (method == "POST")
		_method = Method::POST;
	else if (method == "PUT")
		_method = Method::PUT;
	else if (method == "DELETE")
		_method = Method::DELETE;
	else if (method == "HEAD")
		_method = Method::HEAD;
	else if (method == "OPTIONS")
		_method = Method::OPTIONS;
	else if (method == "TRACE")
		_method = Method::TRACE;
	else if (method == "CONNECT")
		_method = Method::CONNECT;
	else if (method == "PATCH")
		_method = Method::PATCH;
	else
		_method = Method::UNKNOWN;

	stream  >> _path >> _version;
}

void Packet::ParseHeaders(std::string &headers)
{
	std::string key(headers.substr(0, headers.find(':')));
	std::string value(headers.substr(headers.find(':')), headers.size() - 1);

	_headers[key] = value;
}

void Packet::ParseBody(std::string &body)
{
	_body = body;
}

Method Packet::getMethod() { return _method; }
std::string Packet::getPath() { return _path; }
std::string Packet::getVersion() { return _version; }
std::map<std::string, std::string> Packet::getHeaders() { return _headers; }
std::string Packet::getBody() { return _body; }

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

