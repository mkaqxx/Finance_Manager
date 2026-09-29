#include "finance_manager.h"


Finance_manager::Finance_manager() {
    storage.load(accounts, transactions, categories, budgets);
    check_the_regular_expense_date();
}


Finance_manager::~Finance_manager() {
    save_data();
}


std::expected<void, int> Finance_manager::save_data() noexcept {
    return storage.save(accounts, transactions, categories, budgets);

}


const std::vector<Account*> Finance_manager::get_accounts_raw() const {
    std::vector<const Account*> res;
    res.reserve(accounts.size());
    for (const auto& a : accounts) res.push_back(a.get());
    return res;
}


const std::vector<Transaction*> Finance_manager::get_transactions_raw() const {
    std::vector<Transaction*> res;
    res.reserve(transactions.size());
    for (const auto& t : transactions) res.push_back(t.get());
    return res;
}


std::expected<void, int> Finance_manager::add_account(std::unique_ptr<Account> account) {
    accounts.push_back(std::move(account));
     return save_data();
}


std::expected<void, int> Finance_manager::add_transaction(std::unique_ptr<Transaction> transaction) {
    transactions.push_back(std::move(transaction));
    if (auto* trans = dynamic_cast<Transfer *>(transaction.get())) {
        unsigned source_id = trans->get_account_id();
        unsigned dest_id = trans->get_destination_id();
        for (const auto& acc : accounts) {
            if (acc->get_id() == source_id) {
               acc->apply_transaction(*trans);
            }
            if (acc->get_id() == dest_id) {
                acc->apply_transaction(*trans);
            }
        }

        return save_data();
    }
    else {
        unsigned category_id = transaction->get_category_id();
        unsigned account_id = transaction->get_account_id();
        for (const auto& acc : accounts) {
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
        return save_data();
    }
}


std::expected<void, int> Finance_manager::add_category(const Category& category) {
    categories.push_back(category);
    return save_data();
}


std::expected<void, int> Finance_manager::add_budget(const Budget& budget) {
    budgets.push_back(budget);
    return save_data();
}


const Category & Finance_manager::get_category_by_id( unsigned id) const {
    for (const auto &c : categories) {
        if (c.id == id) return c;
    }
    throw std::runtime_error("Category not found");
}


Account *Finance_manager::get_account_by_id(unsigned id) const {
    for (auto& acc : accounts) {
        if (acc->get_id() == id) return acc.get();
    }
    throw std::runtime_error("Account not found");
}


std::expected<void, int> Finance_manager::check_the_regular_expense_date() {
    auto now = std::chrono::system_clock::now();
    std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
    std::vector<std::unique_ptr<Transaction>> to_add;
    for (auto& transaction : transactions) {
        if (auto regular = dynamic_cast<RegularExpense *>(transaction.get())) {
            while (regular->get_next_date() <= ymd) {
                unsigned new_id = transactions.size() + to_add.size() + 1;
                 auto new_expense = std::make_unique<Expense>(new_id, regular->get_amount(), regular->get_next_date(),
                regular->get_category_id(), regular->get_account_id());
                to_add.push_back(std::move(new_expense));
                regular->update_next_date();
            }
        }
    }
    for (auto& t : to_add) {
       return add_transaction(std::move(t));
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
