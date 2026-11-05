#pragma once
#include <string>
#include <chrono>
#include <sstream>
#include "transaction.h"

enum class Currency { //перечисление для валюты
    BYN,
    USD,
    EUR,
    RUB
};

//утилиты для перевода перечисления в строку
std::string CurrencyToString(Currency currency);
Currency StringToCurrency(const std::string &currency);


class Account { //абстрактный класс счет
protected:
    unsigned id; //айди счета
    std::string name; //имя счета
    double balance; //баланс
    Currency currency; //валюта
    public:
    Account(unsigned id, const std::string &name, double balance, Currency currency) :
        id(id), name(name), balance(balance), currency(currency) {} //конструктор
    //виртуальные методы
    virtual std::string get_type() const = 0;
    virtual std::string get_info() const = 0;
    //применение транзакции к балансу счета
    void apply_transaction(Transaction& transaction);
    //геттеры
    unsigned get_id() const { return id; }
    std::string get_name() const { return name; }
    double get_balance() const { return balance; }
    Currency get_currency() const { return currency; }
    //деструктор по умолчанию
    virtual ~Account() = default;
    //сеттеры
    void set_name(std::string new_name) { name = new_name; }
    void set_balance(double new_balance) { balance = new_balance; }
    void set_currency(Currency new_currency) { currency = new_currency; }
};


class CashAccount : public Account { //класс-наследник наличный счет
    public:
    CashAccount(unsigned id, const std::string &name, double balance, Currency currency) :
        Account(id, name, balance, currency) {}
    //полиморфные методы
    std::string get_type() const override { return "cash_account"; }
    std::string get_info() const override;
};


class BankAccount : public Account { //класс-наследник банковский счет
protected:
    std::string last_four_digits; //4 цифры карты
    public:
    BankAccount(unsigned id, const std::string &name, double balance, Currency currency,
        const std::string &last_four_digits) : Account(id, name, balance, currency),
        last_four_digits(last_four_digits) {}
    //полиморфные методы
    std::string get_type() const override { return "bank_account"; }
    std::string get_info() const override;

    std::string get_last_four_digits() const { return last_four_digits; }
    void set_last_four_digits(std::string new_last_four_digits) { last_four_digits = new_last_four_digits; }
};


class SavingsAccount : public Account { //класс-наследник счет-копилка
    protected:
    double goal_amount; //целевой баланс
    std::chrono::year_month_day deadline; //срок
    public:
    SavingsAccount(unsigned id, const std::string &name, double balance, Currency currency, double goal_amount,
        std::chrono::year_month_day deadline) : Account(id, name, balance, currency),
        goal_amount(goal_amount), deadline(deadline) {}
    //полиморфные методы
    std::string get_type() const override { return "savings_account"; }
    std::string get_info() const override;
    //возвращает прогресс накопления
    double get_progress() const { return goal_amount > 0.0 ? (balance / goal_amount * 100.0) : 0.0; }
    //геттеры
    double  get_goal_amount() const { return goal_amount; }
    std::chrono::year_month_day get_deadline() const { return deadline; }
    //расчет, сколько нужно откладывать ежемесячно
    double monthly_required() const;
    //сеттеры
    void set_goal_amount(double new_goal_amount) { goal_amount = new_goal_amount; }
    void set_deadline(std::chrono::year_month_day new_deadline) { deadline = new_deadline; }
};