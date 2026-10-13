#pragma once
#include <string>

enum class ErrorCode {
    Success = 0,
    InvalidInput,
    NotFound,
    InsufficientFunds,
    CurrencyMismatch,
    IoError,
    ParseError,
    Unknown
};

struct AppError {
    ErrorCode code = ErrorCode::Unknown;
    std::string message;

    AppError() = default;
    AppError(ErrorCode code, std::string message) : code(code), message(std::move(message)) {}
    AppError(std::string message) : code(ErrorCode::Unknown), message(std::move(message)) {}
};
