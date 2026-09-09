#pragma once
#include <chrono>
#include <sstream>
#include <string>


std::string ymd_to_string(std::chrono::year_month_day ymd);
std::chrono::year_month_day ymd_from_string(const std::string& str);
std::string ym_to_string(std::chrono::year_month ym);