#include "report.h"
#include "../utils/chrono_to_string.h"





json MonthlyReport::generate() const {
    Statistics stats;
    double income, expense, balance;
    income = stats.total_income(transactions, from, to);
    expense = stats.total_expense(transactions, from, to);
    balance = stats.total_balance(accounts);
    std::vector<CategoryStat> category_stats;
    category_stats = stats.top_categories(transactions, categories, from, to, 3);
    json j;
    j["type"] = "monthly";
    j["period"] = ym_to_string(std::chrono::year_month{from.year(), from.month()});
    j["total_income"] = income;
    j["total_expense"] = expense;
    j["balance"] = balance;
    json c_stats = json::array();
    for (auto category_stat : category_stats) {
        json c;
        c["category_id"] =category_stat.category.id;
        c["name"] = category_stat.category.name;
        c["color"] = category_stat.category.color;
        c["amount"] = category_stat.amount;
        c_stats.push_back(c);
    }
    j["expenses_by_category"] = c_stats;
    return j;
}


json CategoryReport::generate() const {
    Statistics stats;
    double total = stats.total_expense(transactions, from, to);
    std::vector<CategoryStat> category_stats = stats.expenses_by_category(transactions, categories, from, to);
    json j;
    j["type"] = "category";
    j["from"] = ymd_to_string(from);
    j["to"] = ymd_to_string(to);
    json c_stats = json::array();
    for (auto category_stat : category_stats) {
        json c;
        c["category_id"] =category_stat.category.id;
        c["name"] = category_stat.category.name;
        c["color"] = category_stat.category.color;
        c["amount"] = category_stat.amount;
        if (total != 0.0) {
            c["percent"] = category_stat.amount/total*100;
        }
        else c["percent"] = 0.0;
        c_stats.push_back(c);
    }
    j["expenses_by_category"] = c_stats;
    j["total"] = total;
    return j;
}


json YearlyReport::generate() const {
    Statistics stats;
    std::vector<Monthly_sum> monthly_sums;
    monthly_sums = stats.monthly_summary(transactions, year);
    json j;
    j["type"] = "yearly";
    j["year"] = year;
    json months = json::array();
    for (size_t i = 0; i < monthly_sums.size(); i++) {
        json m;
        m["month"] = i+1;
        m["income"] = monthly_sums[i].month_income;
        m["expense"] = monthly_sums[i].month_expense;
        months.push_back(m);
    }
    j["months"] = months;
    return j;
}