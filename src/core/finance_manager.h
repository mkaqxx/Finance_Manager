#pragma once
#include "storage.h"
#include "report.h"
#include <vector>
#include <unordered_map>
#include "../json.hpp"
#include <memory>
using json = nlohmann::json;

class Finance_manager {
    private:
    std::vector<std::unique_ptr<Account>> accounts;
    std::vector<std::unique_ptr<Transaction>> transactions;
    std::vector<Category> categories;
    std::vector<Budget> budgets;
    Storage storage;
    std::unordered_map<unsigned, Account*> account_lookup;
    std::unordered_map<unsigned, size_t> category_lookup;
    bool is_dirty = false;
    void rebuild_lookups();
    public:
    //сохранение
    std::expected<void, AppError> save_data() noexcept;

    //конструктор и деструктор
    Finance_manager();
    ~Finance_manager() noexcept;

    //для RAII
    Finance_manager(const Finance_manager&) = delete;
    Finance_manager& operator=(const Finance_manager&) = delete;

    //Перемещающие конструкторы
    Finance_manager(Finance_manager&&) noexcept = default;
    Finance_manager& operator=(Finance_manager&&) noexcept = default;

    //добавление
    std::expected<void, AppError> add_transaction(std::unique_ptr<Transaction> transaction);
    std::expected<void, AppError> add_account(std::unique_ptr<Account> account);
    std::expected<void, AppError> add_category(const Category& category);
    std::expected<void, AppError> add_budget(const Budget& budget);


    //геттеры сырых указателей
    const std::vector<Account*> get_accounts_raw() const;
    const std::vector<Transaction*> get_transactions_raw() const;
    const std::vector<Category>& get_categories() const { return categories; }
    const std::vector<Budget>& get_budgets() const { return budgets; }

    const Category &get_category_by_id( unsigned id) const;
    Account *get_account_by_id( unsigned id) const;

    std::expected<void, AppError> check_the_regular_expense_date();
    json get_monthly_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const;
    json get_category_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const;
    json get_yearly_report(int year) const;


    std::expected<void, AppError> remove_account(unsigned id);
    std::expected<void, AppError> remove_transaction(unsigned id);
    std::expected<void, AppError> remove_category(unsigned id);
    std::expected<void, AppError> remove_budget(unsigned id);
    std::expected<void, AppError> edit_budget(unsigned id, double new_limit);
    std::expected<void, AppError> edit_account(unsigned id, const std::string& name, double balance, Currency currency,
                                              const std::string& last_four_digits, double goal_amount,
                                              std::chrono::year_month_day deadline);
};
