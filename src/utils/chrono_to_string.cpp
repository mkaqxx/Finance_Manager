#include "chrono_to_string.h"
#include <iomanip>


std::string ymd_to_string(std::chrono::year_month_day ymd) {
    std::ostringstream ss;
    ss << std::setw(4) << std::setfill('0') << static_cast<int>(ymd.year()) << '-'
       << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.month()) << '-'
       << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.day());
    return ss.str();
}


std::chrono::year_month_day ymd_from_string(const std::string &str) {
    std::stringstream ss(str);
    unsigned y, m, d;
    char dash1, dash2;
    if (ss >> y >> dash1 >> m >> dash2 >> d && dash1 == '-' && dash2 == '-') {
        std::chrono::year_month_day ymd{
            std::chrono::year(static_cast<int>(y)),
            std::chrono::month(m),
            std::chrono::day(d)
        };
        if (ymd.ok()) {
            return ymd;
        }
    }
    throw std::runtime_error("Invalid or non-existent calendar date: " + str);
}


std::string ym_to_string(std::chrono::year_month ym) {
    std::ostringstream ss;
    ss << std::setw(4) << std::setfill('0') << static_cast<int>(ym.year()) << '-'
       << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ym.month());
    return ss.str();
}