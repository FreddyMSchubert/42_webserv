#include "Request.hpp"

Request::Request(const std::string &rawPacket)
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
	setStatus(Status::UNKNOWN);
}

/* ----- PARSING ----- */

void Request::ParseRequestLine(std::string &line)
{
	std::string::size_type separator = line.find(' ');
	if (separator == std::string::npos)
		throw std::runtime_error("Invalid packet request line");
	std::string method = line.substr(0, separator);
	std::istringstream stream(line.substr(separator, line.size() - 1));

	if (method == "GET")
		setMethod(Method::GET);
	else if (method == "POST")
		setMethod(Method::POST);
	else if (method == "DELETE")
		setMethod(Method::DELETE);
	else
		setMethod(Method::UNKNOWN);

	std::string path, version;
	stream  >> path >> version;
	if (path.find('?') != std::string::npos)
	{
		std::string::size_type separator = path.find('?');
		setPath(sanitizeUri(path.substr(0, separator)));
		setArgs(path.substr(separator + 1));
	}
	else
		setPath(sanitizeUri(path));
	setVersion(version);
}

void Request::ParseHeaders(std::string &headers)
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

		addHeader(key, value);
	}
}

void Request::ParseBody(std::string &body)
{
	// Chunked Transfer Encoding
	std::map<std::string, std::string> &headers = getHeaders();
	if (headers.find("Transfer-Encoding") != headers.end() && headers["Transfer-Encoding"] == "chunked")
	{
		size_t pos = 0;
		setBody("");

		while (true)
		{
			size_t crlfPos = body.find("\r\n", pos);
			if (crlfPos == std::string::npos)
				throw std::runtime_error("Invalid chunked encoding: Missing CRLF after chunk size");

			std::string chunkSizeStr = body.substr(pos, crlfPos - pos);
			size_t chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
			pos = crlfPos + 2;

			if (chunkSize == 0)
				break;

			if (body.size() < pos + chunkSize)
				throw std::runtime_error("Invalid chunked encoding: Incomplete chunk data");

			std::string chunkData = body.substr(pos, chunkSize);
			appendToBody(chunkData);
			pos += chunkSize;

			if (body.substr(pos, 2) != "\r\n")
				throw std::runtime_error("Invalid chunked encoding: Missing CRLF after chunk data");

			pos += 2;
		}
	}
	// Content-Length header
	else if (headers.find("Content-Length") != headers.end())
	{
		size_t contentLength = std::stoi(headers["Content-Length"]);
		if (body.size() < contentLength)
			throw std::runtime_error("Invalid body size");
		setBody(body.substr(0, contentLength));
	}
	// No specified body encoding
	else
	{
		// throw std::runtime_error("Unsupported transfer encoding");
		setBody(body);
	}
}

std::string Request::sanitizeUri(const std::string& uri)
{
	std::string sanitized_uri = uri;
	size_t pos;
	while ((pos = sanitized_uri.find("..")) != std::string::npos)
		sanitized_uri.erase(pos, 2);
	return sanitized_uri;
}
