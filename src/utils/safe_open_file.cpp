#include "safe_open_file.h"

std::expected<std::ifstream, int> open_in_file(const std::string& file_name) {
    std::ifstream ifs(file_name);
    if (!ifs.is_open()) {
        return std::unexpected(errno);
    }
    return ifs;
}

std::expected<std::ofstream, int> open_out_file(const std::string& file_name) {
    std::ofstream ofs(file_name);
    if (!ofs.is_open()) {
        return std::unexpected(errno);
    }
    return ofs;
}