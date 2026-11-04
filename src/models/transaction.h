#pragma once
#include <chrono>
#include <string>
#include <sstream>


enum class Period { // перечисление для постоянных расходов
    DAILY,
    WEEKLY,
    MONTHLY,
    YEARLY
};

class Transaction { //абстрактный класс
protected:
    unsigned id; // айди транзакции
    double amount; //сумма
    std::chrono::year_month_day date; //дата
    unsigned category_id; //айди категории
    unsigned account_id; //айди аккаунта транзакции
    public:
    //конструктор
    Transaction(unsigned id, double amount, std::chrono::year_month_day date,
        unsigned category_id, unsigned account_id): id(id), amount(amount),
            date(date), category_id(category_id), account_id(account_id) {}
    //виртуальные методы
    virtual void apply_to_balance(double & balance) = 0;
    virtual std::string get_type() const = 0;
    //геттеры
    unsigned get_id() const { return id; }
    double get_amount() const { return amount; }
    unsigned get_category_id() const { return category_id; }
    unsigned get_account_id() const { return account_id; }
    std::chrono::year_month_day get_date() const { return date; }

    virtual ~Transaction() = default; //виртуальный деструктор
};


class Income : public Transaction { //класс-наследник доход
    public:
    Income(unsigned id, double amount, std::chrono::year_month_day date, unsigned category_id,
           unsigned account_id) : Transaction(id, amount, date, category_id, account_id) {}
    void apply_to_balance(double & balance) override{ balance += amount; };
    std::string get_type() const override { return "income"; }
};


class Expense : public Transaction { //класс-наследник расход
    public:
    Expense(unsigned id, double amount, std::chrono::year_month_day date, unsigned category_id,
            unsigned account_id) : Transaction(id, amount, date, category_id, account_id) {
    }
    void apply_to_balance(double & balance) override{ balance -= amount; };
    std::string get_type() const override { return "expense"; }
};


class RegularExpense : public Expense { //класс-наследник регулярный доход
    protected:
    std::chrono::year_month_day next_date; //дата следующего списания
    Period period; //периодичность списания
public:
    RegularExpense(unsigned id, double amount, std::chrono::year_month_day date, unsigned category_id,
        unsigned account_id, std::chrono::year_month_day next_date, Period period)
            : Expense(id, amount, date, category_id, account_id), next_date(next_date), period(period) {
    }
    //копирующий конструктор
    RegularExpense(const RegularExpense& transaction) = default;
    std::string get_type() const override { return "regular_expense"; }
    std::chrono::year_month_day get_next_date() const { return next_date; }
    Period get_period() const { return period; }
    void update_next_date(); //обновление даты следующего списания
};


class Transfer : public Transaction { //класс-наследник перевод
    protected:
    unsigned destination_id; //айди адресата
    public:
    Transfer(unsigned id, double amount, std::chrono::year_month_day date, unsigned account_id,
        unsigned destination_id) : Transaction(id, amount, date, 0, account_id), destination_id(destination_id) {}

    void apply_to_source(double &balance) const { balance -= amount; }; //операция для источника
    void apply_to_destination(double &balance) const { balance += amount; }; //операция для получателя
    std::string get_type() const override { return "transfer"; }
    unsigned get_destination_id() const { return destination_id; }
    void apply_to_balance(double &balance) override{} // заглушка

};


//утилиты для перевода перечисления в строку
std::string Period_to_string(Period period);
Period String_to_period(const std::string &period);
