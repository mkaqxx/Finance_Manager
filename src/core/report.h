#pragma once
#include <vector>
#include <chrono>
#include <string>
#include "statistics.h"
#include "../json.hpp"
using json = nlohmann::json; //псевдоним

class Report { //абстрактный класс отчет
public:
    //виртуальные методы
    virtual json generate() const = 0;
    virtual std::string get_type() const = 0;
    //виртуальный деструктор
    virtual ~Report() = default;
};

class MonthlyReport : public Report { //класс-наследник месячный отчет
    //поля класса для вывода отчетов
    const std::vector<Transaction*>& transactions;
    const std::vector<Account*>& accounts;
    const std::vector<Category>& categories;
    std::chrono::year_month_day from, to;
public:
    //конструктор
    MonthlyReport(const std::vector<Transaction*>& t,
                  const std::vector<Account*>& a,
                  const std::vector<Category>& c,
                  std::chrono::year_month_day from,
                  std::chrono::year_month_day to)
        : transactions(t), accounts(a), categories(c), from(from), to(to) {}
    //полиморфные методы (генерация отчета и геттер типа)
    json generate() const override;
    std::string get_type() const override { return "monthly"; }
};

class CategoryReport : public Report { //класс-наследник отчет по категориям
    //поля класса для вывода отчетов
    const std::vector<Transaction*>& transactions;
    const std::vector<Account*>& accounts;
    const std::vector<Category>& categories;
    std::chrono::year_month_day from, to;
public:
    //конструктор
    CategoryReport(const std::vector<Transaction*>& t,
                   const std::vector<Account*>& a,
                   const std::vector<Category>& c,
                   std::chrono::year_month_day from,
                   std::chrono::year_month_day to)
        : transactions(t), accounts(a), categories(c), from(from), to(to) {}
    //полиморфные методы (генерация отчета и геттер типа)
    json generate() const override;
    std::string get_type() const override { return "category"; }
};

class YearlyReport : public Report { //класс-наследник годовой отчет
    //поля класса для вывода отчетов
    const std::vector<Transaction*>& transactions;
    const std::vector<Account*>& accounts;
    int year;
public:
    //конструктор
    YearlyReport(const std::vector<Transaction*>& t,
                 const std::vector<Account*>& a,
                 int year)
        : transactions(t), accounts(a), year(year) {}
    //полиморфные методы (генерация отчета и геттер типа)
    json generate() const override;
    std::string get_type() const override { return "yearly"; }
};