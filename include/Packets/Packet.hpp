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
	protected:
		Method _method;
		std::string _path;
		std::string _version;
		std::map<std::string, std::string> _headers;
		std::string _body;
	public:
		Packet(Packet const &src);
		Packet &operator=(Packet const &src);
		Packet(Method method, const std::string path, const std::string version, const std::map<std::string, std::string> headers, const std::string body);
		Packet(const std::string &rawPacket);
		~Packet() = default;
		Method getMethod();
		std::string getPath();
		std::string getVersion();
		std::map<std::string, std::string> getHeaders();
		std::string getBody();
		void logData();
		// virtual void Run() = 0;
};
