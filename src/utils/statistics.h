#pragma once
#include "../core/finance_manager.h"


struct CategoryStat {
    Category category;
    double amount;
};


struct monthly_sum {
    double month_income;
    double month_expense;
};

class Statistics {
    public:
    double total_income(const std::vector<Transaction*> &transactions,
        std::chrono::year_month_day start_date, std::chrono::year_month_day end_date) const;

    double total_expense(const std::vector<Transaction*> &transactions,
        std::chrono::year_month_day start_date, std::chrono::year_month_day end_date) const;

    double total_balance(const std::vector<Account *> &accounts) const;

    std::vector<CategoryStat> top_categories(const std::vector<Transaction*> &transactions,
        const std::vector<Category> &categories,
        std::chrono::year_month_day from,
        std::chrono::year_month_day to,
        int n) const;

    std::vector<CategoryStat> expenses_by_category(const std::vector<Transaction *> &transactions,
        const std::vector<Category> &categories,
        std::chrono::year_month_day from,
        std::chrono::year_month_day to) const;

    monthly_sum monthly_summary(const std::vector<Transaction *> &transactions, int year) const;
};