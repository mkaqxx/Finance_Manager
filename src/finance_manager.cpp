#include "finance_manager.h"


Finance_manager::Finance_manager() {
    storage.load(*this);
    check_the_regular_expense_date();
}


Finance_manager::~Finance_manager() {
    storage.save(*this);
    for (auto* acc : accounts) delete acc;
    for (auto* t : transactions) delete t;
}


void Finance_manager::add_account(Account *account) {
    accounts.push_back(account);
    accounts_changed = true;
    storage.save(*this);
}


void Finance_manager::add_transaction(Transaction *transaction) {
    transactions.push_back(transaction);
    if (auto* trans = dynamic_cast<Transfer *>(transaction)) {
        unsigned source_id = trans->get_account_id();
        unsigned dest_id = trans->get_destination_id();
        for (auto* acc : accounts) {
            if (acc->get_id() == source_id) {
               acc->apply_transaction(*trans);
            }
            else if (acc->get_id() == dest_id) {
                acc->apply_transaction(*trans);
            }
        }
        accounts_changed = true;
        transactions_changed = true;
        storage.save(*this);
        return;
    }
    else {
        unsigned category_id = transaction->get_category_id();
        unsigned account_id = transaction->get_account_id();
        for (auto *acc : accounts) {
            if (acc->get_id() == account_id) {
                acc->apply_transaction(*transaction);
            }
        }
        for (auto &budget : budgets) {
            if (budget.get_category().id == category_id) {
                budget.update_amount(transaction->get_amount());
            }
        }
        accounts_changed = true;
        transactions_changed = true;
        budgets_changed = true;
        storage.save(*this);
    }
}


void Finance_manager::add_category(Category category) {
    categories.push_back(category);
    categories_changed = true;
    storage.save(*this);
}


void Finance_manager::add_budget(Budget budget) {
    budgets.push_back(budget);
    budgets_changed = true;
    storage.save(*this);
}


const Category & Finance_manager::get_category_by_id( unsigned id) const {
    for (const auto &c : categories) {
        if (c.id == id) return c;
    }
    throw std::runtime_error("Category not found");
}


void Finance_manager::check_the_regular_expense_date() {
    auto now = std::chrono::system_clock::now();
    std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
    std::vector<Transaction *> to_add;
    for (Transaction *transaction : transactions) {
        if (auto regular = dynamic_cast<RegularExpense *>(transaction)) {
            if (regular->get_next_date() == ymd) {
                RegularExpense* new_transaction = new RegularExpense(*regular);
                new_transaction->update_next_date();
                to_add.push_back(new_transaction);
            }
        }
    }
    for (Transaction* t: to_add) add_transaction(t);
}
