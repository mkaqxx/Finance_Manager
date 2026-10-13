#pragma once
#include <string>

enum class ErrorCode {
    Success = 0,
    FileNotFound,
    IoError,
    ParseError,
    NotFound,
    AlreadyExists,
    InvalidInput,
    Unknown
};

struct AppError {
    ErrorCode code{ErrorCode::Success};
    std::string message;
};
