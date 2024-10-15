#pragma once

#include "Config.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <filesystem>
#include <algorithm>
#include <vector>

std::string getFileAsString(const std::string& path);
bool isAllowedMethodAt(t_server_config &config, std::string path, Method method);
std::vector<std::filesystem::directory_entry> getDirectoryEntries(const std::string& path);
t_location get_location(t_server_config &config, std::string path);
bool isSubroute(const std::string& route, const std::string& subroute);