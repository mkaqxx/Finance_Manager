#include "statistics.h"
#include <algorithm>


Currency Statistics::get_account_currency( const std::vector<Account*>& accounts, unsigned account_id) const{
    for (const auto* acc : accounts) //линейный поиск
        if (acc->get_id() == account_id)
            return acc->get_currency();
    return Currency::BYN; //в случае ошибки поиска
}


double Statistics::income(
    const std::vector<Transaction*>& transactions,
    const std::vector<Account*>& accounts,
    std::chrono::year_month_day from,
    std::chrono::year_month_day to,
    Currency cur) const
{
    double total = 0;
    for (auto* t : transactions) {
        //если доход, совпадают даты и валюта
        if (t->get_type() == "income"
            && t->get_date() >= from
            && t->get_date() <= to
            && get_account_currency(accounts, t->get_account_id()) == cur)
            total += t->get_amount();
    }
    return total;
}


double Statistics::expense(
    const std::vector<Transaction*>& transactions,
    const std::vector<Account*>& accounts,
    std::chrono::year_month_day from,
    std::chrono::year_month_day to,
    Currency cur) const
{
    double total = 0;
    for (auto* t : transactions) {
        //если расход, совпадают даты и валюта
        if ((t->get_type() == "expense" || t->get_type() == "regular_expense")
            && t->get_date() >= from
            && t->get_date() <= to
            && get_account_currency(accounts, t->get_account_id()) == cur)
            total += t->get_amount();
    }
    return total;
}

double Statistics::balance( const std::vector<Account*>& accounts, Currency cur) const {
    double total = 0;
    for (auto* acc : accounts)
        //если совпадает валюта
        if (acc->get_currency() == cur)
            total += acc->get_balance();
    return total;
}


std::vector<CategoryStat> Statistics::expenses_by_category(
    const std::vector<Transaction*>& transactions,
    const std::vector<Account*>& accounts,
    const std::vector<Category>& categories,
    std::chrono::year_month_day from,
    std::chrono::year_month_day to,
    Currency cur) const
{
    std::vector<CategoryStat> result; //вектор пар
    for (auto& cat : categories) { //цикл по категориям
        double amount = 0; //сумма по текущей категории
        for (auto* t : transactions) { //перебор всех транзакций
            //если совпадает айди категории, дата, валюта и это расход
            if (t->get_category_id() == cat.id
                && t->get_date() >= from
                && t->get_date() <= to
                && (t->get_type() == "expense" || t->get_type() == "regular_expense")
                && get_account_currency(accounts, t->get_account_id()) == cur)
                amount += t->get_amount();
        }
        //добавление новой пары
        result.emplace_back(cat, amount);
    }
    return result;
}



std::vector<CategoryStat> Statistics::top_categories(
    const std::vector<Transaction*>& transactions,
    const std::vector<Account*>& accounts,
    const std::vector<Category>& categories,
    std::chrono::year_month_day from,
    std::chrono::year_month_day to,
    Currency cur,
    int n) const
{
    if (n <= 0) return {}; //если нужно вернутьь <=0 категорий
    //получение всей статистики трат по категориям
    auto result = expenses_by_category(transactions, accounts, categories, from, to, cur);
    //сортировка по убыванию суммы (через std::sort и лямбду)
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.amount > b.amount;
    });
    //срез вектора до нужного размера
    if (static_cast<size_t>(n) < result.size()) {
        result.resize(static_cast<size_t>(n));
    }
    return result;
}



std::vector<Monthly_sum> Statistics::monthly_summary(
    const std::vector<Transaction*>& transactions,
    const std::vector<Account*>& accounts,
    int year,
    Currency cur) const
{
    std::vector<Monthly_sum> result(12); //12 месяцев
    //цикл по всем транзакциям
    for (auto* t : transactions) {
        //если не совпадает год
        if (static_cast<int>(t->get_date().year()) != year) continue;
        //если не совпадает валюта
        if (get_account_currency(accounts, t->get_account_id()) != cur) continue;
        //получение месяца текущей транзакции
        unsigned month = static_cast<unsigned>(t->get_date().month());
        if (t->get_type() == "income") //если дохож
            result[month - 1].month_income += t->get_amount();
        else if (t->get_type() == "expense" || t->get_type() == "regular_expense")
            result[month - 1].month_expense += t->get_amount();
    }
    return result;
}
