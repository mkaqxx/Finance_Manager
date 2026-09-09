#pragma once
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include "statistics.h"
#include "../json.hpp"
using json = nlohmann::json;


class Report {
    public:
    virtual json generate() const =0;
    virtual std::string get_type() const =0;
    virtual ~Report() = default;
};


class MonthlyReport :public Report {
    private:
    const std::vector<Transaction *>& transactions;
    const std::vector<Account*>& accounts;
    const std::vector<Category>& categories;
    std::chrono::year_month_day from, to;
    public:
    MonthlyReport(const std::vector<Transaction *> &t,
        const std::vector<Account*> &a,
        const std::vector<Category> &c,
        std::chrono::year_month_day from,
        std::chrono::year_month_day to) : transactions(t), accounts(a), categories(c), from(from), to(to) {}
    json generate() const override;
    std::string get_type() const override {return "monthly";}
};


class CategoryReport :public Report {
private:
    const std::vector<Transaction *>& transactions;
    const std::vector<Category>& categories;
    std::chrono::year_month_day from, to;
    public:
    CategoryReport(const std::vector<Transaction *> &t,
        const std::vector<Category> &c,
        std::chrono::year_month_day from,
        std::chrono::year_month_day to) : transactions(t), categories(c), from(from), to(to) {}
    json generate() const override;
    std::string get_type() const override {return "category";}
};



class YearlyReport :public Report {
    private:
    const std::vector<Transaction *>& transactions;
    int year;
    public:
    YearlyReport(const std::vector<Transaction *> &t, int year) : transactions(t), year(year) {}
    json generate() const override;
    std::string get_type() const override {return "yearly";}
};


