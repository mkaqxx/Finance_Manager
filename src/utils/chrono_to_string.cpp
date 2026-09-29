#include "chrono_to_string.h"


std::string ymd_to_string(std::chrono::year_month_day ymd) {
    std::stringstream ss;
    ss<<ymd;
    return ss.str();
}


std::chrono::year_month_day ymd_from_string(const std::string &str) {
    std::stringstream ss(str);
    unsigned y, m, d;
    char dash1, dash2;
    if (ss >> y >> dash1 >> m >> dash2 >> d && dash1 == '-' && dash2 == '-') {
        std::chrono::year_month_day ymd{
            std::chrono::year(y),
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
    std::stringstream ss;
    ss<<ym;
    return ss.str();
}