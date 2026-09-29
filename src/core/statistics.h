#pragma once
#include "../models/account.h"
#include "../models/budget.h"
#include <chrono>
#include <vector>

struct CategoryStat {
    Category category;
    double amount;
};

struct Monthly_sum {
    double month_income = 0.0;
    double month_expense = 0.0;
};

class Statistics {
    // вспомогательный метод — найти валюту счёта по id
    Currency get_account_currency(
        const std::vector<Account*>& accounts,
        unsigned account_id) const;

public:
    double income(
        const std::vector<Transaction*>& transactions,
        const std::vector<Account*>& accounts,
        std::chrono::year_month_day from,
        std::chrono::year_month_day to,
        Currency cur) const;

    double expense(
        const std::vector<Transaction*>& transactions,
        const std::vector<Account*>& accounts,
        std::chrono::year_month_day from,
        std::chrono::year_month_day to,
        Currency cur) const;

    double balance(
        const std::vector<Account*>& accounts,
        Currency cur) const;

    // расходы по категориям для одной валюты
    std::vector<CategoryStat> expenses_by_category(
        const std::vector<Transaction*>& transactions,
        const std::vector<Account*>& accounts,
        const std::vector<Category>& categories,
        std::chrono::year_month_day from,
        std::chrono::year_month_day to,
        Currency cur) const;

    // топ N категорий для одной валюты
    std::vector<CategoryStat> top_categories(
        const std::vector<Transaction*>& transactions,
        const std::vector<Account*>& accounts,
        const std::vector<Category>& categories,
        std::chrono::year_month_day from,
        std::chrono::year_month_day to,
        Currency cur,
        int n) const;

    // помесячная статистика для одной валюты
    std::vector<Monthly_sum> monthly_summary(
        const std::vector<Transaction*>& transactions,
        const std::vector<Account*>& accounts,
        int year,
        Currency cur) const;
};