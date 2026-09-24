#pragma once
#include <string>
enum class TransactionType {
    INCOME,
    EXPENSE
};


enum class Status {
    NORMAL,
    WARNING,
    EXCEEDED
};


std::string t_type_to_string(const TransactionType &type);
TransactionType t_type_from_string( const std::string &type);
std::string status_to_string(const Status &status);
Status status_from_string(const std::string &status);


struct Category {
    unsigned id;
    TransactionType type;
    std::string name;
    std::string color; //ui

};


class Budget {
    protected:
    Category category;
    double limit;
    double current_amount;
    public:
    Budget( Category &category, double limit, double current_amount):
    category(category), limit(limit), current_amount(current_amount) {}
    void update_amount(double amount) { current_amount += amount; }
    Status get_status() const;
    Category get_category() const { return category; }
    double get_limit() const { return limit; }
    double get_current_amount() const { return current_amount; }
    void set_limit(double new_limit) { limit = new_limit; }
};