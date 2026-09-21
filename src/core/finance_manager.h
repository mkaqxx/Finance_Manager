#pragma once
#include "storage.h"
#include "report.h"
#include <vector>
#include "../json.hpp"
using json = nlohmann::json;

class Finance_manager {
    private:
    std::vector<Account*> accounts;
    std::vector<Transaction*> transactions;
    std::vector<Category> categories;
    std::vector<Budget> budgets;
    Storage storage;
    public:
    void save_data();
    Finance_manager();
    ~Finance_manager();
    void add_transaction(Transaction *transaction);
    void add_account(Account *account);
    void add_category(const Category& category);
    void add_budget(const Budget& budget);
    const std::vector<Account*>& get_accounts() const { return accounts; }
    const std::vector<Transaction*>& get_transactions() const { return transactions; }
    const std::vector<Category>& get_categories() const { return categories; }
    const std::vector<Budget>& get_budgets() const { return budgets; }
    const Category &get_category_by_id( unsigned id) const;
    Account *get_account_by_id( unsigned id) const;
    void check_the_regular_expense_date();
    json get_monthly_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const;
    json get_category_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const;
    json get_yearly_report(int year) const;
    double get_monthly_savings_requirement(unsigned account_id) const;
    double get_balance() const;
    double get_expenses_for_month();
    double get_incomes_for_month();
    void remove_account(unsigned id);
    void remove_transaction(unsigned id);
    void remove_category(unsigned id);
    void remove_budget(unsigned id);
};
