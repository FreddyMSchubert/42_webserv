#pragma once

#include "Config.hpp"
#include "Path.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

std::string getFileAsString(const std::string& path);
bool isAllowedMethodAt(t_server_config &config, Path path, Method method);
t_location get_location(t_server_config &config, std::string path);
bool isSubroute(const std::string& route, const std::string& subroute);
void setNonBlocking(int fd);