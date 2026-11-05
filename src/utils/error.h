#pragma once
#include <string>

enum class ErrorCode { //коды ошибок
    Success = 0,
    InvalidInput,
    NotFound,
    InsufficientFunds,
    CurrencyMismatch,
    IoError,
    ParseError,
    Unknown
};

struct AppError { //ошибка
    ErrorCode code = ErrorCode::Unknown;
    std::string message;

    //конструкторы
    AppError() = default;
    AppError(ErrorCode code, std::string message) : code(code), message(std::move(message)) {}
    AppError(std::string message) : code(ErrorCode::Unknown), message(std::move(message)) {}
};
