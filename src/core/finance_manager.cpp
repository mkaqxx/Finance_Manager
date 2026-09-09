#include "finance_manager.h"


Finance_manager::Finance_manager() {
    storage.load(accounts, transactions, categories, budgets);
    check_the_regular_expense_date();
}


Finance_manager::~Finance_manager() {
    save_data();
    for (auto* acc : accounts) delete acc;
    for (auto* t : transactions) delete t;
}


void Finance_manager::save_data() {
    storage.save(accounts, transactions, categories, budgets);
}


void Finance_manager::add_account(Account *account) {
    accounts.push_back(account);
    save_data();
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
        save_data();
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
        std::string type = transaction->get_type();
            if (type == "expense" || type == "regular_expense"){
                for (auto& budget : budgets) {
                    if (budget.get_category().id == category_id) {
                        budget.update_amount(transaction->get_amount());
                    }
                }
        }
        save_data();
    }
}


void Finance_manager::add_category(const Category& category) {
    categories.push_back(category);
    save_data();
}


void Finance_manager::add_budget(const Budget& budget) {
    budgets.push_back(budget);
    save_data();
}


const Category & Finance_manager::get_category_by_id( unsigned id) const {
    for (const auto &c : categories) {
        if (c.id == id) return c;
    }
    throw std::runtime_error("Category not found");
}


Account *Finance_manager::get_account_by_id(unsigned id) const {
    for (auto *acc : accounts) {
        if (acc->get_id() == id) return acc;
    }
    throw std::runtime_error("Account not found");
}


double Finance_manager::get_monthly_savings_requirement(unsigned account_id) const {
    Account* acc = get_account_by_id(account_id);
    if (auto* savings_acc = dynamic_cast<SavingsAccount*>(acc)) {
        return savings_acc->monthly_required();
    }
    throw std::invalid_argument("Account is not a savings account");
}


void Finance_manager::check_the_regular_expense_date() {
    auto now = std::chrono::system_clock::now();
    std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
    std::vector<Transaction *> to_add;
    for (Transaction *transaction : transactions) {
        if (auto regular = dynamic_cast<RegularExpense *>(transaction)) {
            while (regular->get_next_date() <= ymd) {
                unsigned new_id = transactions.size() + to_add.size() + 1;
                Expense *new_expense = new Expense(new_id, regular->get_amount(), regular->get_next_date(),
                regular->get_category_id(), regular->get_account_id());
                to_add.push_back(new_expense);
                regular->update_next_date();
            }
        }
    }
    for (Transaction * t : to_add) {
        add_transaction(t);
    }
}

json Finance_manager::get_monthly_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const {
    MonthlyReport report(transactions, accounts, categories, from, to);
    return report.generate();
}

json Finance_manager::get_category_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const {
    CategoryReport report(transactions, categories, from, to);
    return report.generate();
}

json Finance_manager::get_yearly_report(int year) const {
    YearlyReport report(transactions, year);
    return report.generate();
}


double Finance_manager::get_balance() const {
    Statistics stats;
    return stats.total_balance(accounts);
}


double Finance_manager::get_expenses_for_month() {
    auto now = std::chrono::system_clock::now();
    auto days = std::chrono::time_point_cast<std::chrono::days>(now);
    std::chrono::year_month_day today{days};
    Statistics stats;
    return stats.total_expense(transactions, today, today);
}


double Finance_manager::get_incomes_for_month() {
    auto now = std::chrono::system_clock::now();
    auto days = std::chrono::time_point_cast<std::chrono::days>(now);
    std::chrono::year_month_day today{days};
    Statistics stats;
    return stats.total_income(transactions, today, today);
}
