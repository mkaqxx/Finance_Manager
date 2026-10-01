#pragma once
#include <chrono>
#include <string>
#include <sstream>


enum class Period {
    DAILY,
    WEEKLY,
    MONTHLY,
    YEARLY
};

class Transaction {
protected:
    unsigned id;
    double amount;
    std::chrono::year_month_day date;
    unsigned category_id;
    unsigned account_id;
    public:
    Transaction(unsigned id, double amount, std::chrono::year_month_day date,
        unsigned category_id, unsigned account_id): id(id), amount(amount),
            date(date), category_id(category_id), account_id(account_id) {}
    virtual void apply_to_balance(double & balance) = 0;
    virtual std::string get_type() const = 0;
    unsigned get_id() const { return id; }
    double get_amount() const { return amount; }
    unsigned get_category_id() const { return category_id; }
    unsigned get_account_id() const { return account_id; }
    std::chrono::year_month_day get_date() const { return date; }
    virtual ~Transaction() = default;
};


class Income : public Transaction {
    public:
    Income(unsigned id, double amount, std::chrono::year_month_day date, unsigned category_id,
           unsigned account_id) : Transaction(id, amount, date, category_id, account_id) {}
    void apply_to_balance(double & balance) override{ balance += amount; };
    std::string get_type() const override { return "income"; }
};


class Expense : public Transaction {
    public:
    Expense(unsigned id, double amount, std::chrono::year_month_day date, unsigned category_id,
            unsigned account_id) : Transaction(id, amount, date, category_id, account_id) {
    }
    void apply_to_balance(double & balance) override{ balance -= amount; };
    std::string get_type() const override { return "expense"; }
};


class RegularExpense : public Expense {
    protected:
    std::chrono::year_month_day next_date;
    Period period;
public:
    RegularExpense(unsigned id, double amount, std::chrono::year_month_day date, unsigned category_id,
        unsigned account_id, std::chrono::year_month_day next_date, Period period)
            : Expense(id, amount, date, category_id, account_id), next_date(next_date), period(period) {
    }
    RegularExpense(const RegularExpense& transaction) = default;
    std::string get_type() const override { return "regular_expense"; }
    std::chrono::year_month_day get_next_date() const { return next_date; }
    Period get_period() const { return period; }
    void update_next_date();
};


class Transfer : public Transaction {
    protected:
    unsigned destination_id;
    public:
    Transfer(unsigned id, double amount, std::chrono::year_month_day date, unsigned account_id,
        unsigned destination_id) : Transaction(id, amount, date, 0, account_id), destination_id(destination_id) {}
    void apply_to_source(double &balance) const { balance -= amount; };
    void apply_to_destination(double &balance) const { balance += amount; };
    std::string get_type() const override { return "transfer"; }
    unsigned get_destination_id() const { return destination_id; }
    void apply_to_balance(double &balance) override{} // заглушка

};

std::string Period_to_string(Period period);
Period String_to_period(const std::string &period);
