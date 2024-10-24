#pragma once

#include "Packet.hpp"
#include "Request.hpp"
#include "Config.hpp"
#include "Path.hpp"

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
			void handle_file_req(t_server_config &config, Request &req);
			void handle_dir_req(t_server_config &config, Request &req);
		void handlePost(Request& req, t_server_config &config);
		void handleDelete(Request& req, t_server_config &config);

	public:
		Response(Request &req, t_server_config &config);
		~Response() = default;

		// methods
		std::string getRawPacket();
		std::string getMimeType(std::string filename);
};
