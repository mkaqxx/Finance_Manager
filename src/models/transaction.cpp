#include "transaction.h"

void RegularExpense::update_next_date() {
    switch (period) {
        case Period::DAILY: next_date = std::chrono::year_month_day{
            std::chrono::sys_days{next_date} + std::chrono::days{1}}; break;
        case Period::WEEKLY: next_date = std::chrono::year_month_day{
            std::chrono::sys_days{next_date} + std::chrono::days{7}}; break;
        case Period::MONTHLY: {
            auto ym = std::chrono::year_month{next_date.year(), next_date.month()} + std::chrono::months{1};
            next_date = ym / std::min(next_date.day(), (ym / std::chrono::last).day());
            break;
        }
        case Period::YEARLY: {
            auto ym = std::chrono::year_month{next_date.year(), next_date.month()} + std::chrono::years{1};
            next_date = ym / std::min(next_date.day(), (ym / std::chrono::last).day());
            break;
        }
        default: break;
    }
}





std::string Period_to_string(Period period) {
    switch (period) {
        case Period::DAILY: return "daily";
        case Period::WEEKLY: return "weekly";
        case Period::MONTHLY: return "monthly";
        case Period::YEARLY: return "yearly";
        default : throw std::runtime_error("Invalid period");
    }
}


Period String_to_period(const std::string &period) {
    if (period == "daily") return Period::DAILY;
    else if (period == "weekly") return Period::WEEKLY;
    else if (period == "monthly") return Period::MONTHLY;
    else if (period == "yearly") return Period::YEARLY;
    else throw std::runtime_error("Invalid period: " + period);
}
