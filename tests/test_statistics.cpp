#include <gtest/gtest.h>
#include "core/statistics.h"
#include <chrono>

using namespace std::chrono_literals;

TEST(StatisticsTest, TotalBalanceByCurrency) {
    Statistics stats;

    std::vector<std::unique_ptr<Account>> acc_ptrs;
    acc_ptrs.push_back(std::make_unique<CashAccount>(1, "Кошелек BYN 1", 500.0, Currency::BYN));
    acc_ptrs.push_back(std::make_unique<CashAccount>(2, "Кошелек BYN 2", 300.0, Currency::BYN));
    acc_ptrs.push_back(std::make_unique<CashAccount>(3, "Кошелек USD", 1000.0, Currency::USD));

    std::vector<Account*> raw_accs;
    for (auto& a : acc_ptrs) raw_accs.push_back(a.get());

    // Баланс в BYN: 500 + 300 = 800
    EXPECT_DOUBLE_EQ(stats.balance(raw_accs, Currency::BYN), 800.0);
    // Баланс в USD: 1000
    EXPECT_DOUBLE_EQ(stats.balance(raw_accs, Currency::USD), 1000.0);
    // Баланс в EUR: 0
    EXPECT_DOUBLE_EQ(stats.balance(raw_accs, Currency::EUR), 0.0);
}

TEST(StatisticsTest, FilterByDateAndCurrency) {
    Statistics stats;

    std::vector<std::unique_ptr<Account>> acc_ptrs;
    acc_ptrs.push_back(std::make_unique<CashAccount>(1, "Счет BYN", 1000.0, Currency::BYN));
    acc_ptrs.push_back(std::make_unique<CashAccount>(2, "Счет USD", 1000.0, Currency::USD));

    std::vector<Account*> raw_accs;
    for (auto& a : acc_ptrs) raw_accs.push_back(a.get());

    std::vector<std::unique_ptr<Transaction>> tx_ptrs;
    // Январь: 100 BYN расход
    tx_ptrs.push_back(std::make_unique<Expense>(1, 100.0, std::chrono::year_month_day{2026y, std::chrono::January, 15d}, 1, 1));
    // Февраль: 200 BYN расход
    tx_ptrs.push_back(std::make_unique<Expense>(2, 200.0, std::chrono::year_month_day{2026y, std::chrono::February, 15d}, 1, 1));
    // Февраль: 50 USD расход (в другой валюте!)
    tx_ptrs.push_back(std::make_unique<Expense>(3, 50.0, std::chrono::year_month_day{2026y, std::chrono::February, 15d}, 1, 2));

    std::vector<Transaction*> raw_txs;
    for (auto& t : tx_ptrs) raw_txs.push_back(t.get());

    // Фильтр только на Февраль 2026 в валюте BYN:
    auto from = std::chrono::year_month_day{2026y, std::chrono::February, 1d};
    auto to   = std::chrono::year_month_day{2026y, std::chrono::February, 28d};

    double feb_byn_expense = stats.expense(raw_txs, raw_accs, from, to, Currency::BYN);
    EXPECT_DOUBLE_EQ(feb_byn_expense, 200.0);

    double feb_usd_expense = stats.expense(raw_txs, raw_accs, from, to, Currency::USD);
    EXPECT_DOUBLE_EQ(feb_usd_expense, 50.0);
}
