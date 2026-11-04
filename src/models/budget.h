#pragma once
#include <string>
enum class TransactionType { //тип транзакции
    INCOME,
    EXPENSE
};


enum class Status { //статус лимита
    NORMAL,
    WARNING,
    EXCEEDED
};

//утилиты для перечисления
std::string t_type_to_string(const TransactionType &type);
TransactionType t_type_from_string( const std::string &type);
std::string status_to_string(const Status &status);
Status status_from_string(const std::string &status);


struct Category { //категория операций
    unsigned id; //айди
    TransactionType type; //тип
    std::string name; //имя
    std::string color; //цвет отображения

};


class Budget { //лимит по категории
    protected:
    Category category; //категория лимита
    double limit; //сумма лимита
    double current_amount; //текущие траты
    public:
    //конструктор
    Budget( Category &category, double limit, double current_amount):
    category(category), limit(limit), current_amount(current_amount) {}
    //обновление текущей потраченной суммы
    void update_amount(double amount) { current_amount += amount; }
    //геттеры
    Status get_status() const;
    Category get_category() const { return category; }
    double get_limit() const { return limit; }
    double get_current_amount() const { return current_amount; }
    //для редактирования
    void set_limit(double new_limit) { limit = new_limit; }
};