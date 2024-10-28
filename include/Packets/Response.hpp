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
		void handleGet(Request& req, t_server_config &config);
			void handle_file_req(t_server_config &config, FilePath &path);
			void handle_dir_req(t_server_config &config, Path &path);
		void handlePost(Request& req, t_server_config &config);
		void handleDelete(Request& req, t_server_config &config);

	public:
		Response(Request &req, t_server_config &config);
		~Response() = default;

		// methods
		std::string getRawPacket();
		static std::string getMimeType(std::string filename);
};
