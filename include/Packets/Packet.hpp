#pragma once

#include <iostream>
#include <map>
#include <string>
#include "../Enums.hpp"

class Packet
{
	private:
		void ParseRequestLine(std::string &line);
		void ParseHeaders(std::string &headers);
		void ParseBody(std::string &body);
		std::string sanitizeUri(const std::string& uri);
	protected:
		Method _method;
		std::string _path;
		std::string _version;
		std::map<std::string, std::string> _headers;
		std::string _body;
		Status _status;
		bool _is_empty;
	public:
		Packet();
		Packet(const Packet &src);
		Packet &operator=(const Packet &src);
		Packet(Method method, const std::string path, const std::string version, const std::map<std::string, std::string> headers, const std::string body, Status status);
		Packet(const std::string &rawPacket);
		virtual ~Packet();
		std::string getPath();
		void setPath(const std::string path);
		std::string getVersion();
		void setVersion(const std::string version);
		std::map<std::string, std::string> getHeaders();
		void setHeaders(const std::map<std::string, std::string> headers);
		void addHeader(const std::string key, const std::string value);
		std::string getBody();
		void setBody(const std::string body);
		bool isEmpty();
		Method getMethod();
		void setMethod(Method method);
		void setStatus(Status status);
		Status getStatus();
		void logData();
		virtual void Run();
};
