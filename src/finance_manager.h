#pragma once
#include "storage.h"
#include <vector>

class Finance_manager {
    private:
    std::vector<Account*> accounts;
    std::vector<Transaction*> transactions;
    std::vector<Category> categories;
    std::vector<Budget> budgets;
    Storage storage;
    void save_data();
    public:
    Finance_manager();
    ~Finance_manager();
    void add_transaction(Transaction *transaction);
    void add_account(Account *account);
    void add_category(Category category);
    void add_budget(Budget budget);
    std::vector<Account*> get_accounts() const { return accounts; }
    std::vector<Transaction*> get_transactions() const { return transactions; }
    std::vector<Category> get_categories() const { return categories; }
    std::vector<Budget> get_budgets() const { return budgets; }
    const Category &get_category_by_id( unsigned id) const;
    Account *get_account_by_id( unsigned id) const;
    void check_the_regular_expense_date();
};
