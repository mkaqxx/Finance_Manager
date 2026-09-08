#include "transaction.h"


void RegularExpense::update_next_date() {
    switch (period) {
        case Period::DAILY: next_date = std::chrono::year_month_day{
            std::chrono::sys_days{next_date} + std::chrono::days{1}}; break;
        case Period::WEEKLY: next_date = std::chrono::year_month_day{
            std::chrono::sys_days{next_date} + std::chrono::days{7}}; break;
        case Period::MONTHLY: next_date += std::chrono::months{1}; break;
        case Period::YEARLY: next_date += std::chrono::years{1}; break;
        default: break;
    }
}


std::string ymd_to_string(std::chrono::year_month_day ymd) {
    std::stringstream ss;
    ss<<ymd;
    return ss.str();
}


std::chrono::year_month_day ymd_from_string(std::string str) {
    std::stringstream ss(str);
    unsigned y, m, d;
    char dash1, dash2;
    if (ss>>y>>dash1>>m>>dash2>>d && dash1 == dash2 && dash2 == '-') {
        return std::chrono::year_month_day{std::chrono::year(y), std::chrono::month(m), std::chrono::day(d)};
    }
    throw std::runtime_error("Invalid format");
}


std::string Period_to_string(Period period) {
    switch (period) {
        case Period::DAILY: return "daily";
        case Period::WEEKLY: return "weekly";
        case Period::MONTHLY: return "monthly";
        case Period::YEARLY: return "yearly";
    }
}


Period String_to_period(const std::string &period) {
    if (period == "daily") return Period::DAILY;
    else if (period == "weekly") return Period::WEEKLY;
    else if (period == "monthly") return Period::MONTHLY;
    else return Period::YEARLY; 
}
