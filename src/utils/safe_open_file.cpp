#include "safe_open_file.h"

std::expected<std::ifstream, AppError> open_in_file(const std::string& file_name) {
    std::ifstream ifs(file_name);
    if (!ifs.is_open()) {
        return std::unexpected(AppError{ErrorCode::IoError, "Не удалось открыть файл для чтения: " + file_name});
    }
    return ifs;
}

std::expected<std::ofstream, AppError> open_out_file(const std::string& file_name) {
    std::ofstream ofs(file_name);
    if (!ofs.is_open()) {
        return std::unexpected(AppError{ErrorCode::IoError, "Не удалось открыть файл для записи: " + file_name});
    }
    return ofs;
}