#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include "../Enums.hpp"

class Packet
{
	private:
		std::string _path;
		std::string _version;
		std::map<std::string, std::string> _headers;
		std::string _body;
		Method _method;
		Status _status;
	public:
		// constructors
		Packet();
		Packet(Method method, const std::string path, const std::string version, const std::map<std::string, std::string> headers, const std::string body, Status status);
		Packet(const Packet &src);
		Packet &operator=(const Packet &src);
		virtual ~Packet() = default;

		// methods
		void logData();

		// getters and setters
		std::string getPath();
		void setPath(const std::string path);

		std::string getVersion();
		void setVersion(const std::string version);

		std::map<std::string, std::string> &getHeaders();
		void setHeaders(const std::map<std::string, std::string> headers);
		void addHeader(const std::string key, const std::string value);

		std::string getBody();
		void setBody(const std::string body);
		void appendToBody(const std::string bodyAppend);

		Method getMethod();
		void setMethod(Method method);

		void setStatus(Status status);
		Status getStatus();
};
