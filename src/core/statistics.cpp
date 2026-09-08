#include "statistics.h"


double Statistics::total_income(const std::vector<Transaction *> &transactions,
                                std::chrono::year_month_day start_date, std::chrono::year_month_day end_date) const {
    double total_income = 0;
    for (auto *t: transactions) {
        if (t->get_type() == "income" && t->get_date() >= start_date && t->get_date() <= end_date)
            total_income += t->get_amount();
    }
    return total_income;
}


double Statistics::total_expense(const std::vector<Transaction *> &transactions,
    std::chrono::year_month_day start_date, std::chrono::year_month_day end_date) const {
    double total_expense = 0;
    for (auto *t: transactions) {
        if ((t->get_type() == "expense" || t->get_type() == "regular_expense") && t->get_date() >= start_date && t->get_date() <= end_date)
            total_expense += t->get_amount();
    }
    return total_expense;
}


double Statistics::total_balance(const std::vector<Account *> &accounts) const {
    double total_balance = 0;
    for (auto *account: accounts) {
        total_balance += account->get_balance();
    }
    return total_balance;
}


std::vector<CategoryStat> Statistics::expenses_by_category(const std::vector<Transaction *> &transactions,
    const std::vector<Category> &categories,
    std::chrono::year_month_day from,
    std::chrono::year_month_day to) const {
    std::vector<CategoryStat> category_stats;
    for (auto& category: categories) {
        category_stats.emplace_back(category, 0.0);
        for (auto& transaction: transactions) {
            if (transaction->get_category_id() == category.id
                && transaction->get_date() >= from
                && transaction->get_date() <= to
                && (transaction->get_type() == "expense" || transaction->get_type() == "regular_expense")) {

                category_stats.back().amount += transaction->get_amount();
            }
        }
    }
    return category_stats;
}


std::vector<CategoryStat> Statistics::top_categories(const std::vector<Transaction *> &transactions,
    const std::vector<Category> &categories,
    std::chrono::year_month_day from,
    std::chrono::year_month_day to, int n) const {

    std::vector<CategoryStat> category_stats =expenses_by_category(transactions, categories, from, to);
    int size =category_stats.size();
    for (int i = 0; i < size; i++) {
        int max = i;
        for (int j = i + 1; j < size; j++) {
            if (category_stats[j].amount>category_stats[max].amount) {
                max = j;
            }
        }
        auto temp = category_stats[i];
        category_stats[i] = category_stats[max];
        category_stats[max] = temp;
    }

    if (n<category_stats.size()) {
        category_stats.resize(n);
    }
    return category_stats;
}


std::vector<Monthly_sum> Statistics::monthly_summary(const std::vector<Transaction *> &transactions, int year) const {
    std::vector<Monthly_sum> monthly_summary(12);
    for (auto *transaction: transactions) {
        if (static_cast<int>(transaction->get_date().year()) == year) {
            if (transaction->get_type() == "income") {
                unsigned month = static_cast<unsigned>(transaction->get_date().month());
                monthly_summary[month - 1].month_income += transaction->get_amount();
            }
            else if (transaction->get_type() == "expense" || transaction->get_type() =="regular_expense") {
                unsigned month = static_cast<unsigned>(transaction->get_date().month());
                monthly_summary[month - 1].month_expense += transaction->get_amount();
            }
        }
    }
    return monthly_summary;
}
