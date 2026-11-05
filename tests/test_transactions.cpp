#include <gtest/gtest.h>
#include "models/transaction.h"
#include <chrono>

using namespace std::chrono_literals;

// ============================================================================
// ТЕСТОВАЯ ГРУППА 1: Базовые транзакции (Income, Expense, Transfer)
// ============================================================================

TEST(TransactionTest, IncomeAttributesAndType) {
    std::chrono::year_month_day date{2026y, std::chrono::October, 15d};
    Income inc(1, 450.0, date, 2, 10);

    EXPECT_EQ(inc.get_id(), 1u);
    EXPECT_DOUBLE_EQ(inc.get_amount(), 450.0);
    EXPECT_EQ(inc.get_date(), date);
    EXPECT_EQ(inc.get_category_id(), 2u);
    EXPECT_EQ(inc.get_account_id(), 10u);
    EXPECT_EQ(inc.get_type(), "income");
}

TEST(TransactionTest, ExpenseAttributesAndType) {
    std::chrono::year_month_day date{2026y, std::chrono::October, 16d};
    Expense exp(2, 85.0, date, 3, 10);

    EXPECT_EQ(exp.get_id(), 2u);
    EXPECT_DOUBLE_EQ(exp.get_amount(), 85.0);
    EXPECT_EQ(exp.get_type(), "expense");
}

TEST(TransactionTest, TransferAppliesToSourceAndDestination) {
    std::chrono::year_month_day date{2026y, std::chrono::October, 20d};
    // Перевод со счёта 10 на счёт 20 на сумму 150
    Transfer transfer(3, 150.0, date, 10, 20);

    EXPECT_EQ(transfer.get_type(), "transfer");
    EXPECT_EQ(transfer.get_account_id(), 10u);
    EXPECT_EQ(transfer.get_destination_id(), 20u);

    double src_balance = 500.0;
    double dest_balance = 100.0;

    transfer.apply_to_source(src_balance);
    transfer.apply_to_destination(dest_balance);

    EXPECT_DOUBLE_EQ(src_balance, 350.0);
    EXPECT_DOUBLE_EQ(dest_balance, 250.0);
}

// ============================================================================
// ТЕСТОВАЯ ГРУППА 2: Регулярные платежи (RegularExpense) и расчет дат
// ============================================================================

TEST(RegularExpenseTest, DailyPeriodAdvance) {
    std::chrono::year_month_day start_date{2026y, std::chrono::March, 10d};
    RegularExpense reg(1, 20.0, start_date, 1, 1, start_date, Period::DAILY);

    EXPECT_EQ(reg.get_next_date(), start_date);

    reg.update_next_date();
    std::chrono::year_month_day expected{2026y, std::chrono::March, 11d};
    EXPECT_EQ(reg.get_next_date(), expected);
}

TEST(RegularExpenseTest, WeeklyPeriodAdvance) {
    std::chrono::year_month_day start_date{2026y, std::chrono::March, 10d};
    RegularExpense reg(1, 50.0, start_date, 1, 1, start_date, Period::WEEKLY);

    reg.update_next_date();
    std::chrono::year_month_day expected{2026y, std::chrono::March, 17d};
    EXPECT_EQ(reg.get_next_date(), expected);
}

TEST(RegularExpenseTest, MonthlyPeriodAdvanceStandard) {
    std::chrono::year_month_day start_date{2026y, std::chrono::January, 15d};
    RegularExpense reg(1, 100.0, start_date, 1, 1, start_date, Period::MONTHLY);

    reg.update_next_date();
    std::chrono::year_month_day expected{2026y, std::chrono::February, 15d};
    EXPECT_EQ(reg.get_next_date(), expected);
}

TEST(RegularExpenseTest, MonthlyPeriodAdvanceEndOfMonthClamping) {
    // 31 января -> следующий месяц февраль (в 2026 году 28 дней)
    std::chrono::year_month_day start_date{2026y, std::chrono::January, 31d};
    RegularExpense reg(1, 100.0, start_date, 1, 1, start_date, Period::MONTHLY);

    reg.update_next_date();
    // Логика min(next_date.day(), last_day) должна ограничить дату 28 февраля
    std::chrono::year_month_day expected{2026y, std::chrono::February, 28d};
    EXPECT_EQ(reg.get_next_date(), expected);
}

TEST(RegularExpenseTest, YearlyPeriodAdvance) {
    std::chrono::year_month_day start_date{2026y, std::chrono::May, 5d};
    RegularExpense reg(1, 500.0, start_date, 1, 1, start_date, Period::YEARLY);

    reg.update_next_date();
    std::chrono::year_month_day expected{2027y, std::chrono::May, 5d};
    EXPECT_EQ(reg.get_next_date(), expected);
}

// ============================================================================
// ТЕСТОВАЯ ГРУППА 3: Конвертация периодичности (Period)
// ============================================================================

TEST(PeriodTest, PeriodStringRoundtrip) {
    EXPECT_EQ(Period_to_string(Period::DAILY), "daily");
    EXPECT_EQ(Period_to_string(Period::WEEKLY), "weekly");
    EXPECT_EQ(Period_to_string(Period::MONTHLY), "monthly");
    EXPECT_EQ(Period_to_string(Period::YEARLY), "yearly");

    EXPECT_EQ(String_to_period("daily"), Period::DAILY);
    EXPECT_EQ(String_to_period("weekly"), Period::WEEKLY);
    EXPECT_EQ(String_to_period("monthly"), Period::MONTHLY);
    EXPECT_EQ(String_to_period("yearly"), Period::YEARLY);
}

TEST(PeriodTest, InvalidPeriodThrows) {
    EXPECT_THROW(String_to_period("hourly"), std::runtime_error);
    EXPECT_THROW(String_to_period(""), std::runtime_error);
}
