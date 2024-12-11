#pragma once

#include "Config.hpp"
#include "Path.hpp"
#include "FilePath.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

std::vector<std::filesystem::directory_entry> getDirectoryEntries(const std::string& path);
t_location get_location(Config &config, std::string path);
std::variant<Path, FilePath> createPath(const std::string &path, Path::Type type, Config &config);
std::string getFileData(std::string path);
