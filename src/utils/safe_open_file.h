#pragma once

#include <expected>
#include <fstream>
#include <system_error>


std::expected<std::ifstream, int> open_in_file(const std::string& file_name);
std::expected<std::ofstream, int> open_out_file(const std::string& file_name);