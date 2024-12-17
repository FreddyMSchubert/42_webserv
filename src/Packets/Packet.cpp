#include "Packet.hpp"

/* ----- CONSTRUCTORS ----- */

Packet::Packet()
{
	_method = Method::UNKNOWN;
	_path = "";
	_version = "";
	_headers.clear();
	_body = "";
	_status = Status::UNKNOWN;
}

Packet::Packet(Method method, const std::string path, const std::string version, const std::map<std::string, std::string> headers, const std::string body, Status status)
{
	_method = method;
	_path = path;
	_version = version;
	_headers = headers;
	_body = body;
	_status = status;
}

Packet::Packet(const Packet &src)
{
	_headers.clear();

	_method = src._method;
	_path = src._path;
	_version = src._version;
	_headers = src._headers;
	_body = src._body;
}

Packet &Packet::operator=(const Packet &src)
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

/* ----- METHODS ----- */

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
	if (!_body.empty())
		std::cout << "Body: " << _body << std::endl;
}

std::string Packet::getRawPacket()
{
	std::string rawData;

	if (getStatus() == Status::UNKNOWN)
		setStatus(Status::OK); // TODO: Probably dont do this this sounds like a terrible idea

	rawData +=  getVersion() + " " + std::to_string((int)getStatus()) + "\r\n";

	for (auto &header : getHeaders())
		rawData += header.first + ": " + header.second + "\r\n";
	
	rawData += "\r\n" + getBody();
	rawData += "\r\n\r\n";

	return rawData;
}

/* ----- GETTERS & SETTERS ----- */

void Packet::setPath(const std::string path) { _path = path; }
std::string Packet::getPath() { return _path; }

void Packet::setArgs(const std::string args) { _args = args; }
std::string Packet::getArgs() { return _args; }

void Packet::setVersion(const std::string version) { _version = version; }
std::string Packet::getVersion() { return _version; }

void Packet::setHeaders(const std::map<std::string, std::string> headers) { _headers = headers; }
std::map<std::string, std::string> & Packet::getHeaders() { return _headers; }
void Packet::addHeader(const std::string key, const std::string value) { _headers[key] = value; }

void Packet::setBody(const std::string body) { _body = body; }
std::string Packet::getBody() { return _body; }
void Packet::appendToBody(const std::string bodyAppend) { _body += bodyAppend; }

Method Packet::getMethod() { return _method; }
void Packet::setMethod(Method method) { _method = method; }

void Packet::setStatus(Status status) { _status = status; }
Status Packet::getStatus() { return _status; }
