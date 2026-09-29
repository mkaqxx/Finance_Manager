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
            if (acc->get_id() == dest_id) {
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
                if (get_account_by_id(account_id)->get_currency() == Currency::BYN) {
                    for (auto& budget : budgets) {
                        if (budget.get_category().id == category_id) {
                            budget.update_amount(transaction->get_amount());
                        }
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
    CategoryReport report(transactions, accounts, categories, from, to);
    return report.generate();
}

json Finance_manager::get_yearly_report(int year) const {
    YearlyReport report(transactions, accounts, year);
    return report.generate();
}


void Finance_manager::remove_account(unsigned id) {
    for (size_t i = 0; i < accounts.size(); ++i) {
        if (accounts[i]->get_id() == id) {
            delete accounts[i];
            accounts.erase(accounts.begin() + i); break;
        }
    }

    for (auto it = transactions.begin(); it != transactions.end(); ) {
        if ((*it)->get_account_id() == id) {
            delete *it; // Очищаем память объекта
            it = transactions.erase(it);
        } else {
            ++it;
        }
    }
    save_data();
}

void Finance_manager::remove_category(unsigned id) {
    // 1. Удаляем категорию
    for (auto it = categories.begin(); it != categories.end();++it ) {
        if (it->id == id) {
            it = categories.erase(it);
            break;
        }
    }

    // 2. Удаляем все связанные с ней бюджеты
    for (auto it = budgets.begin(); it != budgets.end(); ++it) {
        if (it->get_category().id == id) {
            it = budgets.erase(it);
            break;
        }
    }

    for (auto it = transactions.begin(); it != transactions.end(); ) {
        if ((*it)->get_category_id() == id) {
            delete *it; // Очищаем память объекта
            it = transactions.erase(it);
        } else {
            ++it;
        }
    }

    save_data();
}

void Finance_manager::remove_budget(unsigned id) {
    for (auto it = budgets.begin(); it != budgets.end(); ++it) {
        if (it->get_category().id == id) {
            it = budgets.erase(it);
            break;
        }
    }

    save_data();
}

void Finance_manager::remove_transaction(unsigned id) {
    for (auto it = transactions.begin(); it != transactions.end(); ++it) {
        if ((*it)->get_id() == id) {
            Transaction* tx = *it;
            if (tx->get_type() == "transfer") {
                auto* transfer = dynamic_cast<Transfer*>(tx);
                Account* src = get_account_by_id(transfer->get_account_id());
                Account* dest = get_account_by_id(transfer->get_destination_id());
                Income reversal_income(0, tx->get_amount(), tx->get_date(), 0, 0);
                src->apply_transaction(reversal_income);
                Expense reversal_expense(0, tx->get_amount(), tx->get_date(), 0, 0);
                dest->apply_transaction(reversal_expense);
            }
            else {
                Account* acc = get_account_by_id(tx->get_account_id());
                if (tx->get_type() == "expense" || tx->get_type() == "regular_expense") {
                    Income reversal_income(0, tx->get_amount(), tx->get_date(), 0, 0);
                    acc->apply_transaction(reversal_income);
                    if (acc->get_currency()== Currency::BYN) {
                        for (auto& budget : budgets) {
                            if (budget.get_category().id == tx->get_category_id()) {
                                budget.update_amount(-tx->get_amount());
                            }
                        }
                    }
                }
                else if (tx->get_type() == "income") {
                    Expense reversal_expense(0, tx->get_amount(), tx->get_date(), 0, 0);
                    acc->apply_transaction(reversal_expense);
                }
            }
            delete tx;
            transactions.erase(it);
            break;
        }
    }
    save_data();
}


void Finance_manager::edit_budget(unsigned id, double new_limit) {
    for (auto &budget : budgets) {
        if (budget.get_category().id == id) {
            budget.set_limit(new_limit);
            break;
        }
    }
    save_data();
}
