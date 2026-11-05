#include <gtest/gtest.h>
#include "models/account.h"
#include <chrono>

using namespace std::chrono_literals;

// ============================================================================
// ТЕСТОВАЯ ГРУППА 1: Базовые операции со счетами и балансом
// ============================================================================

TEST(AccountTest, InitialValuesAreCorrect) {
    CashAccount acc(1, "Наличные", 500.0, Currency::BYN);

    EXPECT_EQ(acc.get_id(), 1u);
    EXPECT_EQ(acc.get_name(), "Наличные");
    EXPECT_DOUBLE_EQ(acc.get_balance(), 500.0);
    EXPECT_EQ(acc.get_currency(), Currency::BYN);
    EXPECT_EQ(acc.get_type(), "cash_account");
}

TEST(AccountTest, ApplyIncomeIncreasesBalance) {
    CashAccount acc(1, "Кошелек", 100.0, Currency::BYN);
    Income inc(10, 50.0, std::chrono::year_month_day{2026y, std::chrono::November, 1d}, 1, 1);

    acc.apply_transaction(inc);

    // 100 + 50 = 150
    EXPECT_DOUBLE_EQ(acc.get_balance(), 150.0);
}

TEST(AccountTest, ApplyExpenseDecreasesBalance) {
    CashAccount acc(1, "Кошелек", 300.0, Currency::BYN);
    Expense exp(11, 75.5, std::chrono::year_month_day{2026y, std::chrono::November, 1d}, 1, 1);

    acc.apply_transaction(exp);

    // 300 - 75.5 = 224.5
    EXPECT_DOUBLE_EQ(acc.get_balance(), 224.5);
}

TEST(AccountTest, BankAccountHoldsLastFourDigits) {
    BankAccount card(2, "Зарплатная карта", 1500.0, Currency::USD, "4321");

    EXPECT_EQ(card.get_type(), "bank_account");
    EXPECT_EQ(card.get_last_four_digits(), "4321");

    card.set_last_four_digits("9999");
    EXPECT_EQ(card.get_last_four_digits(), "9999");
}

// ============================================================================
// ТЕСТОВАЯ ГРУППА 2: Накопительные счета (SavingsAccount) и дедлайны
// ============================================================================

TEST(SavingsAccountTest, ProgressCalculation) {
    // Дедлайн через 10 месяцев от текущей даты
    auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    std::chrono::year_month_day future_date{today + std::chrono::days{300}};

    // Цель 1000, баланс 250 -> прогресс 25%
    SavingsAccount savings(3, "Копилка на отпуск", 250.0, Currency::EUR, 1000.0, future_date);

    EXPECT_DOUBLE_EQ(savings.get_progress(), 25.0);

    // Пополняем еще на 250 -> прогресс 50%
    Income inc(1, 250.0, std::chrono::year_month_day{2026y, std::chrono::November, 1d}, 0, 3);
    savings.apply_transaction(inc);
    EXPECT_DOUBLE_EQ(savings.get_progress(), 50.0);
}

TEST(SavingsAccountTest, ProgressWithZeroGoal) {
    auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    std::chrono::year_month_day future_date{today + std::chrono::days{300}};

    SavingsAccount savings(4, "Пустая копилка", 100.0, Currency::BYN, 0.0, future_date);
    // Деление на ноль должно предотвращаться логикой класса -> возвращать 0.0
    EXPECT_DOUBLE_EQ(savings.get_progress(), 0.0);
}

TEST(SavingsAccountTest, MonthlyRequiredReturnsZeroWhenGoalReached) {
    auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    std::chrono::year_month_day future_date{today + std::chrono::days{180}};

    // Баланс 1200 при цели 1000 -> ежемесячно больше откладывать не требуется (0.0)
    SavingsAccount savings(5, "Достигнутая цель", 1200.0, Currency::USD, 1000.0, future_date);
    EXPECT_DOUBLE_EQ(savings.monthly_required(), 0.0);
}

// ============================================================================
// ТЕСТОВАЯ ГРУППА 3: Конвертация строк и валют (Currency)
// ============================================================================

TEST(CurrencyTest, ConversionToString) {
    EXPECT_EQ(CurrencyToString(Currency::BYN), "byn");
    EXPECT_EQ(CurrencyToString(Currency::USD), "usd");
    EXPECT_EQ(CurrencyToString(Currency::EUR), "eur");
    EXPECT_EQ(CurrencyToString(Currency::RUB), "rub");
}

TEST(CurrencyTest, ConversionFromString) {
    EXPECT_EQ(StringToCurrency("byn"), Currency::BYN);
    EXPECT_EQ(StringToCurrency("usd"), Currency::USD);
    EXPECT_EQ(StringToCurrency("eur"), Currency::EUR);
    EXPECT_EQ(StringToCurrency("rub"), Currency::RUB);
}

TEST(CurrencyTest, InvalidCurrencyThrowsException) {
    EXPECT_THROW(StringToCurrency("bitcoin"), std::runtime_error);
    EXPECT_THROW(StringToCurrency(""), std::runtime_error);
}
