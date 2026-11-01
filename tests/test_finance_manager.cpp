#include <gtest/gtest.h>
#include "core/finance_manager.h"
#include <filesystem>

using namespace std::chrono_literals;

// Тестовый класс с изоляцией дисковых данных:
// чтобы тесты не перетирали реальный data.json пользователя,
// мы работаем во временном каталоге или очищаем файл
class FinanceManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Очищаем тестовый файл data.json перед каждым тестом
        std::error_code ec;
        std::filesystem::remove("data.json", ec);
        std::filesystem::remove("data.json.tmp", ec);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove("data.json", ec);
        std::filesystem::remove("data.json.tmp", ec);
    }
};

// ============================================================================
// ТЕСТОВАЯ ГРУППА: Управление счетами (Account Management)
// ============================================================================

TEST_F(FinanceManagerTest, AddAccountSuccessAndLookup) {
    Finance_manager fm;
    auto acc = std::make_unique<CashAccount>(1, "Основной кошелек", 1000.0, Currency::BYN);

    auto res = fm.add_account(std::move(acc));
    EXPECT_TRUE(res.has_value());

    Account* found = fm.get_account_by_id(1);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->get_id(), 1u);
    EXPECT_EQ(found->get_name(), "Основной кошелек");
    EXPECT_DOUBLE_EQ(found->get_balance(), 1000.0);
}

TEST_F(FinanceManagerTest, AddNullAccountFails) {
    Finance_manager fm;
    auto res = fm.add_account(nullptr);

    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::InvalidInput);
}

TEST_F(FinanceManagerTest, RemoveAccountDeletesLinkedTransactions) {
    Finance_manager fm;
    fm.add_account(std::make_unique<CashAccount>(1, "Счет 1", 500.0, Currency::BYN));
    fm.add_account(std::make_unique<CashAccount>(2, "Счет 2", 300.0, Currency::BYN));

    // Добавляем перевод между 1 и 2
    fm.add_transaction(std::make_unique<Transfer>(100, 100.0, std::chrono::year_month_day{2026y, std::chrono::November, 1d}, 1, 2));

    EXPECT_EQ(fm.get_transactions_raw().size(), 1u);

    // Удаляем счет 1 -> транзакция с ним должна удалиться
    auto res = fm.remove_account(1);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(fm.get_account_by_id(1), nullptr);
    EXPECT_EQ(fm.get_transactions_raw().size(), 0u);
}

TEST_F(FinanceManagerTest, RemoveNonExistingAccountReturnsNotFound) {
    Finance_manager fm;
    auto res = fm.remove_account(999);

    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error().code, ErrorCode::NotFound);
}

// ============================================================================
// ТЕСТОВАЯ ГРУППА: Транзакции и балансы
// ============================================================================

TEST_F(FinanceManagerTest, ExpenseDeductsBalance) {
    Finance_manager fm;
    fm.add_account(std::make_unique<CashAccount>(1, "Карта", 1000.0, Currency::BYN));

    // Списание 250
    auto exp = std::make_unique<Expense>(1, 250.0, std::chrono::year_month_day{2026y, std::chrono::November, 1d}, 1, 1);
    auto res = fm.add_transaction(std::move(exp));

    EXPECT_TRUE(res.has_value());
    Account* acc = fm.get_account_by_id(1);
    ASSERT_NE(acc, nullptr);
    EXPECT_DOUBLE_EQ(acc->get_balance(), 750.0);
}

TEST_F(FinanceManagerTest, RemoveTransactionRollsBackBalance) {
    Finance_manager fm;
    fm.add_account(std::make_unique<CashAccount>(1, "Карта", 1000.0, Currency::BYN));

    // Списание 400 -> баланс 600
    fm.add_transaction(std::make_unique<Expense>(10, 400.0, std::chrono::year_month_day{2026y, std::chrono::November, 1d}, 1, 1));
    EXPECT_DOUBLE_EQ(fm.get_account_by_id(1)->get_balance(), 600.0);

    // Удаляем транзакцию 10 -> баланс должен вернуться к 1000
    auto res = fm.remove_transaction(10);
    EXPECT_TRUE(res.has_value());
    EXPECT_DOUBLE_EQ(fm.get_account_by_id(1)->get_balance(), 1000.0);
}

// ============================================================================
// ТЕСТОВАЯ ГРУППА: Бюджеты и статусы
// ============================================================================

TEST_F(FinanceManagerTest, BudgetStatusesCalculation) {
    Category cat{1, TransactionType::EXPENSE, "Продукты", "#00ff00"};
    // Лимит 100
    Budget b(cat, 100.0, 50.0);
    EXPECT_EQ(b.get_status(), Status::NORMAL); // 50%

    b.update_amount(35.0); // 85 / 100 = 85%
    EXPECT_EQ(b.get_status(), Status::WARNING); // >= 80%

    b.update_amount(20.0); // 105 / 100 = 105%
    EXPECT_EQ(b.get_status(), Status::EXCEEDED); // >= 100%
}
