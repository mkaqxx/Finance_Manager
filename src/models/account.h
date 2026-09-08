#pragma once
#include "transaction.h"

enum class Currency {
    BYN,
    USD,
    EUR,
    RUB
};


std::string CurrencyToString(Currency currency);
Currency StringToCurrency(const std::string &currency);
class Account {
protected:
    unsigned id;
    std::string name;
    double balance;
    Currency currency;
    public:
    Account(unsigned id, const std::string &name, double balance, Currency currency) :
        id(id), name(name), balance(balance), currency(currency) {}
    virtual std::string get_type() const = 0;
    virtual std::string get_info() const = 0;
    void apply_transaction(Transaction& transaction);
    unsigned get_id() const { return id; }
    std::string get_name() const { return name; }
    double get_balance() const { return balance; }
    Currency get_currency() const { return currency; }
    virtual ~Account() = default;
};


class CashAccount : public Account {
    public:
    CashAccount(unsigned id, const std::string &name, double balance, Currency currency) :
        Account(id, name, balance, currency) {}
    std::string get_type() const override { return "cash_account"; }
    std::string get_info() const override;
};


class BankAccount : public Account {
protected:
    std::string last_four_digits;
    public:
    BankAccount(unsigned id, const std::string &name, double balance, Currency currency,
        const std::string &last_four_digits) : Account(id, name, balance, currency),
        last_four_digits(last_four_digits) {}
    std::string get_type() const override { return "bank_account"; }
    std::string get_info() const override;
    std::string get_last_four_digits() const { return last_four_digits; }
};


class SavingsAccount : public Account {
    protected:
    double goal_amount;
    std::chrono::year_month_day deadline;
    public:
    SavingsAccount(unsigned id, const std::string &name, double balance, Currency currency, double goal_amount,
        std::chrono::year_month_day deadline) : Account(id, name, balance, currency),
        goal_amount(goal_amount), deadline(deadline) {}
    std::string get_type() const override { return "savings_account"; }
    std::string get_info() const override;
    double get_progress() const { return balance / goal_amount * 100.0; }
    double  get_goal_amount() const { return goal_amount; }
    std::chrono::year_month_day get_deadline() const { return deadline; }
};