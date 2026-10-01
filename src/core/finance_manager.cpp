#include "finance_manager.h"
#include <algorithm>

Finance_manager::Finance_manager() {
    (void)storage.load(accounts, transactions, categories, budgets);
    rebuild_lookups();
    (void)check_the_regular_expense_date();
}

Finance_manager::~Finance_manager() noexcept {
    (void)save_data();
}

void Finance_manager::rebuild_lookups() {
    account_lookup.clear();
    for (const auto& a : accounts) {
        if (a) account_lookup[a->get_id()] = a.get();
    }
    category_lookup.clear();
    for (size_t i = 0; i < categories.size(); ++i) {
        category_lookup[categories[i].id] = i;
    }
}

std::expected<void, int> Finance_manager::save_data() noexcept {
    if (!is_dirty) return {};
    auto res = storage.save(accounts, transactions, categories, budgets);
    if (res) is_dirty = false;
    return res;
}

const std::vector<Account*> Finance_manager::get_accounts_raw() const {
    std::vector<Account*> res;
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
    if (!account) return std::unexpected(-1);
    account_lookup[account->get_id()] = account.get();
    accounts.push_back(std::move(account));
    is_dirty = true;
    return save_data();
}

std::expected<void, int> Finance_manager::add_transaction(std::unique_ptr<Transaction> transaction) {
    if (!transaction) return std::unexpected(-1);
    Transaction* raw_tx = transaction.get();

    if (auto* trans = dynamic_cast<Transfer *>(raw_tx)) {
        unsigned source_id = trans->get_account_id();
        unsigned dest_id = trans->get_destination_id();
        for (const auto& acc : accounts) {
            if (acc->get_id() == source_id || acc->get_id() == dest_id) {
               acc->apply_transaction(*trans);
            }
        }
        transactions.push_back(std::move(transaction));
        is_dirty = true;
        return save_data();
    }
    else {
        unsigned category_id = raw_tx->get_category_id();
        unsigned account_id = raw_tx->get_account_id();
        for (const auto& acc : accounts) {
            if (acc->get_id() == account_id) {
                acc->apply_transaction(*raw_tx);
            }
        }
        std::string type = raw_tx->get_type();
        if (type == "expense" || type == "regular_expense") {
            try {
                Account* acc = get_account_by_id(account_id);
                if (acc && acc->get_currency() == Currency::BYN) {
                    for (auto& budget : budgets) {
                        if (budget.get_category().id == category_id) {
                            budget.update_amount(raw_tx->get_amount());
                        }
                    }
                }
            } catch (...) {}
        }
        transactions.push_back(std::move(transaction));
        is_dirty = true;
        return save_data();
    }
}

std::expected<void, int> Finance_manager::add_category(const Category& category) {
    categories.push_back(category);
    category_lookup[category.id] = categories.size() - 1;
    is_dirty = true;
    return save_data();
}

std::expected<void, int> Finance_manager::add_budget(const Budget& budget) {
    budgets.push_back(budget);
    is_dirty = true;
    return save_data();
}

const Category & Finance_manager::get_category_by_id(unsigned id) const {
    auto it = category_lookup.find(id);
    if (it != category_lookup.end() && it->second < categories.size()) {
        return categories[it->second];
    }
    throw std::runtime_error("Category not found: " + std::to_string(id));
}

Account *Finance_manager::get_account_by_id(unsigned id) const {
    auto it = account_lookup.find(id);
    return (it != account_lookup.end()) ? it->second : nullptr;
}
    
    
    std::expected<void, int> Finance_manager::check_the_regular_expense_date() {
        auto now = std::chrono::system_clock::now();
        std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
        std::vector<std::unique_ptr<Transaction>> to_add;
    
        unsigned max_id = 0;
        for (const auto& t : transactions) {
            if (t->get_id() > max_id) max_id = t->get_id();
        }
    
        for (auto& transaction : transactions) {
            if (auto regular = dynamic_cast<RegularExpense *>(transaction.get())) {
                while (regular->get_next_date() <= ymd) {

                    unsigned new_id = ++max_id;
                    auto new_expense = std::make_unique<Expense>(new_id, regular->get_amount(), regular->get_next_date(),
                        regular->get_category_id(), regular->get_account_id());
                    to_add.push_back(std::move(new_expense));
                    regular->update_next_date();
                }
            }
        }
        for (auto& t : to_add) {
            auto res = add_transaction(std::move(t));
            if (!res) {
                return std::unexpected(res.error());
            }
        }
        return {};
    }
    
    json Finance_manager::get_monthly_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const {
        auto txs = get_transactions_raw();
        auto accs = get_accounts_raw();
        MonthlyReport report(txs, accs, categories, from, to);
        return report.generate();
    }
    
    json Finance_manager::get_category_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const {
        auto txs = get_transactions_raw();
        auto accs = get_accounts_raw();
        CategoryReport report(txs, accs, categories, from, to);
        return report.generate();
    }
    
    json Finance_manager::get_yearly_report(int year) const {
        auto txs = get_transactions_raw();
        auto accs = get_accounts_raw();
        YearlyReport report(txs, accs, year);
        return report.generate();
    }
    
    std::expected<void, int> Finance_manager::remove_account(unsigned id) {
        // 1. Проверяем существование счёта ДО модификаций
        auto it_acc = std::find_if(accounts.begin(), accounts.end(), [id](const auto& a) {
            return a->get_id() == id;
        });
        if (it_acc == accounts.end()) {
            return std::unexpected(-1);
        }

        const Currency acc_currency = (*it_acc)->get_currency();

        // 2. Откатываем связанные переводы на контрагентских счетах и бюджеты
        for (const auto& t : transactions) {
            if (auto* trans = dynamic_cast<Transfer*>(t.get())) {
                if (trans->get_account_id() == id && trans->get_destination_id() != id) {
                    if (Account* dest = get_account_by_id(trans->get_destination_id())) {
                        Expense rollback(0, trans->get_amount(), trans->get_date(), 0, 0);
                        dest->apply_transaction(rollback);
                    }
                } else if (trans->get_destination_id() == id && trans->get_account_id() != id) {
                    if (Account* src = get_account_by_id(trans->get_account_id())) {
                        Income rollback(0, trans->get_amount(), trans->get_date(), 0, 0);
                        src->apply_transaction(rollback);
                    }
                }
            } else if (t->get_account_id() == id) {
                if (t->get_type() == "expense" || t->get_type() == "regular_expense") {
                    if (acc_currency == Currency::BYN) {
                        for (auto& budget : budgets) {
                            if (budget.get_category().id == t->get_category_id()) {
                                budget.update_amount(-t->get_amount());
                            }
                        }
                    }
                }
            }
        }

        // 3. Линейное удаление всех привязанных транзакций за O(N)
        std::erase_if(transactions, [id](const auto& t) {
            if (t->get_account_id() == id) return true;
            if (auto* trans = dynamic_cast<Transfer*>(t.get())) {
                if (trans->get_destination_id() == id) return true;
            }
            return false;
        });

        // 4. Удаляем счёт из кэша и вектора
        account_lookup.erase(id);
        accounts.erase(it_acc);

        is_dirty = true;
        return save_data();
    }
    
    std::expected<void, int> Finance_manager::remove_category(unsigned id) {
        // 1. Откатываем балансы счетов для всех транзакций удаляемой категории в памяти
        for (const auto& t : transactions) {
            if (t->get_category_id() == id) {
                if (Account* acc = get_account_by_id(t->get_account_id())) {
                    if (t->get_type() == "expense" || t->get_type() == "regular_expense") {
                        Income rev(0, t->get_amount(), t->get_date(), 0, 0);
                        acc->apply_transaction(rev);
                    } else if (t->get_type() == "income") {
                        Expense rev(0, t->get_amount(), t->get_date(), 0, 0);
                        acc->apply_transaction(rev);
                    }
                }
            }
        }

        // 2. Удаляем все транзакции этой категории за один проход O(N)
        std::erase_if(transactions, [id](const auto& t) {
            return t->get_category_id() == id;
        });

        // 3. Удаляем связанные бюджеты за O(N)
        std::erase_if(budgets, [id](const auto& b) {
            return b.get_category().id == id;
        });

        // 4. Удаляем саму категорию
        std::erase_if(categories, [id](const auto& c) {
            return c.id == id;
        });

        // Обновляем хэш-таблицу категорий
        category_lookup.clear();
        for (size_t i = 0; i < categories.size(); ++i) {
            category_lookup[categories[i].id] = i;
        }

        is_dirty = true;
        return save_data();
    }
    
    std::expected<void, int> Finance_manager::remove_budget(unsigned id) {
        std::erase_if(budgets, [id](const auto& b) {
            return b.get_category().id == id;
        });
        is_dirty = true;
        return save_data();
    }
    
    std::expected<void, int> Finance_manager::remove_transaction(unsigned id) {
        for (auto it = transactions.begin(); it != transactions.end(); ++it) {
            if ((*it)->get_id() == id) {
                Transaction* tx = it->get();
                if (tx->get_type() == "transfer") {
                    if (auto* transfer = dynamic_cast<Transfer*>(tx)) {
                        try {
                            Account* src = get_account_by_id(transfer->get_account_id());
                            Account* dest = get_account_by_id(transfer->get_destination_id());
                            Income reversal_income(0, tx->get_amount(), tx->get_date(), 0, 0);
                            if (src) src->apply_transaction(reversal_income);
                            Expense reversal_expense(0, tx->get_amount(), tx->get_date(), 0, 0);
                            if (dest) dest->apply_transaction(reversal_expense);
                        } catch (...) {}
                    }
                }
                else {
                    try {
                        Account* acc = get_account_by_id(tx->get_account_id());
                        if (acc) {
                            if (tx->get_type() == "expense" || tx->get_type() == "regular_expense") {
                                Income reversal_income(0, tx->get_amount(), tx->get_date(), 0, 0);
                                acc->apply_transaction(reversal_income);
                                if (acc->get_currency() == Currency::BYN) {
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
                    } catch (...) {}
                }
                transactions.erase(it);
                break;
            }
        }
        is_dirty = true;
        return save_data();
    }
    
    std::expected<void, int> Finance_manager::edit_budget(unsigned id, double new_limit) {
        for (auto &budget : budgets) {
            if (budget.get_category().id == id) {
                budget.set_limit(new_limit);
                break;
            }
        }
        is_dirty = true;
        return save_data();
    }