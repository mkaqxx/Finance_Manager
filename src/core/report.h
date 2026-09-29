#pragma once
#include <vector>
#include <chrono>
#include <string>
#include "statistics.h"
#include "../json.hpp"
using json = nlohmann::json;

class Report {
public:
    virtual json generate() const = 0;
    virtual std::string get_type() const = 0;
    virtual ~Report() = default;
};

class MonthlyReport : public Report {
    const std::vector<Transaction*>& transactions;
    const std::vector<Account*>& accounts;
    const std::vector<Category>& categories;
    std::chrono::year_month_day from, to;
public:
    MonthlyReport(const std::vector<Transaction*>& t,
                  const std::vector<Account*>& a,
                  const std::vector<Category>& c,
                  std::chrono::year_month_day from,
                  std::chrono::year_month_day to)
        : transactions(t), accounts(a), categories(c), from(from), to(to) {}
    json generate() const override;
    std::string get_type() const override { return "monthly"; }
};

class CategoryReport : public Report {
    const std::vector<Transaction*>& transactions;
    const std::vector<Account*>& accounts;
    const std::vector<Category>& categories;
    std::chrono::year_month_day from, to;
public:
    CategoryReport(const std::vector<Transaction*>& t,
                   const std::vector<Account*>& a,
                   const std::vector<Category>& c,
                   std::chrono::year_month_day from,
                   std::chrono::year_month_day to)
        : transactions(t), accounts(a), categories(c), from(from), to(to) {}
    json generate() const override;
    std::string get_type() const override { return "category"; }
};

class YearlyReport : public Report {
    const std::vector<Transaction*>& transactions;
    const std::vector<Account*>& accounts;
    int year;
public:
    YearlyReport(const std::vector<Transaction*>& t,
                 const std::vector<Account*>& a,
                 int year)
        : transactions(t), accounts(a), year(year) {}
    json generate() const override;
    std::string get_type() const override { return "yearly"; }
};