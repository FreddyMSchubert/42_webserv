#pragma once

#include "Packet.hpp"
#include "Request.hpp"
#include "Config.hpp"
#include "Path.hpp"
#include "FilePath.hpp"

#include <variant>
#include <iostream>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

class Response : public Packet
{
	private:
		void handleGet(Request& req, Config &config);
			void handle_file_req(Config &config, FilePath &path);
			void handle_dir_req(Config &config, Path &path);
		void handlePost(Request& req, Config &config);
		void handleDelete(Request& req, Config &config);

	public:
		Response(Request &req, Config &config);
		~Response() = default;

		// methods
		std::string getRawPacket();
		static std::string getMimeType(std::string filename);
};
