#pragma once
#include <chrono>
#include <sstream>
#include <string>

//утилиты для перевода времени в строку
std::string ymd_to_string(std::chrono::year_month_day ymd);
std::chrono::year_month_day ymd_from_string(const std::string& str);
std::string ym_to_string(std::chrono::year_month ym);