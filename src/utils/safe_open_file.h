#pragma once

#include <expected>
#include <fstream>
#include "error.h"

//безопасное открытие файла без выброса исключений
std::expected<std::ifstream, AppError> open_in_file(const std::string& file_name);
std::expected<std::ofstream, AppError> open_out_file(const std::string& file_name);