    #include "finance_manager.h"
    
    
    Finance_manager::Finance_manager() {
        (void)storage.load(accounts, transactions, categories, budgets);
        (void)check_the_regular_expense_date();
    }
    
    
    Finance_manager::~Finance_manager() noexcept {
        (void)save_data();
    }
    
    
    std::expected<void, int> Finance_manager::save_data() noexcept {
        return storage.save(accounts, transactions, categories, budgets);
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
        accounts.push_back(std::move(account));
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
    
    
    const Category & Finance_manager::get_category_by_id(unsigned id) const {
        for (const auto &c : categories) {
            if (c.id == id) return c;
        }
        throw std::runtime_error("Category not found");
    }
    
    
    Account *Finance_manager::get_account_by_id(unsigned id) const {
        for (auto& acc : accounts) {
            if (acc->get_id() == id) return acc.get();
        }
        return nullptr;
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
        for (size_t i = 0; i < accounts.size(); ++i) {
            if (accounts[i]->get_id() == id) {
                accounts.erase(accounts.begin() + i);
                break;
            }
        }
    
        for (auto it = transactions.begin(); it != transactions.end(); ) {
            bool match = ((*it)->get_account_id() == id);
            if (auto* trans = dynamic_cast<Transfer*>(it->get())) {
                if (trans->get_destination_id() == id) {
                    match = true;
                }
            }
            if (match) {
                it = transactions.erase(it);
            } else {
                ++it;
            }
        }
        return save_data();
    }
    
std::expected<void, int> Finance_manager::remove_category(unsigned id) {
        // 1. Находим все транзакции этой категории и корректно откатываем их влияние на балансы
        for (auto it = transactions.begin(); it != transactions.end(); ) {
            if ((*it)->get_category_id() == id) {
                unsigned tx_id = (*it)->get_id();
                (void)remove_transaction(tx_id);
                it = transactions.begin(); // Сбрасываем итератор после модификации вектора
            } else {
                ++it;
            }
        }

        // 2. Удаляем связанные бюджеты
        for (auto it = budgets.begin(); it != budgets.end(); ) {
            if (it->get_category().id == id) {
                it = budgets.erase(it);
            } else {
                ++it;
            }
        }

        // 3. Удаляем саму категорию
        for (auto it = categories.begin(); it != categories.end(); ++it) {
            if (it->id == id) {
                categories.erase(it);
                break;
            }
        }

        return save_data();
    }
    
    std::expected<void, int> Finance_manager::remove_budget(unsigned id) {
        for (auto it = budgets.begin(); it != budgets.end(); ) {
            if (it->get_category().id == id) {
                it = budgets.erase(it);
            } else {
                ++it;
            }
        }
    
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
                            src->apply_transaction(reversal_income);
                            Expense reversal_expense(0, tx->get_amount(), tx->get_date(), 0, 0);
                            dest->apply_transaction(reversal_expense);
                        } catch (...) {}
                    }
                }
                else {
                    try {
                        Account* acc = get_account_by_id(tx->get_account_id());
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
                    } catch (...) {}
                }
                transactions.erase(it);
                break;
            }
        }
        return save_data();
    }
    
    std::expected<void, int> Finance_manager::edit_budget(unsigned id, double new_limit) {
        for (auto &budget : budgets) {
            if (budget.get_category().id == id) {
                budget.set_limit(new_limit);
                break;
            }
        }
        return save_data();
    }