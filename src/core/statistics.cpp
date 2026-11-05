#include "statistics.h"
#include <algorithm>
#include <unordered_map>


//поиск валюты счета по id
std::optional<Currency> Statistics::get_account_currency(const std::vector<Account*>& accounts, unsigned account_id) const {
    for (const auto* acc : accounts) { //линейный поиск
        if (acc && acc->get_id() == account_id)
            return acc->get_currency();
    }
    return std::nullopt; //если счет не найден
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
        if (!t) continue;
        //если доход, совпадают даты и валюта
        if (t->get_type() == "income"
            && t->get_date() >= from
            && t->get_date() <= to) {
            auto acc_cur = get_account_currency(accounts, t->get_account_id());
            if (acc_cur.has_value() && *acc_cur == cur)
                total += t->get_amount();
        }
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
        if (!t) continue;
        //если расход, совпадают даты и валюта
        if ((t->get_type() == "expense" || t->get_type() == "regular_expense")
            && t->get_date() >= from
            && t->get_date() <= to) {
            auto acc_cur = get_account_currency(accounts, t->get_account_id());
            if (acc_cur.has_value() && *acc_cur == cur)
                total += t->get_amount();
        }
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
    //карта счетов и их валют для быстрого поиска за O(A)
    std::unordered_map<unsigned, Currency> account_currencies;
    for (const auto* acc : accounts) {
        if (acc) account_currencies[acc->get_id()] = acc->get_currency();
    }

    //подсчет трат по категориям за один проход по транзакциям за O(T)
    std::unordered_map<unsigned, double> category_totals;
    for (const auto* t : transactions) {
        if (!t) continue;
        if (t->get_type() != "expense" && t->get_type() != "regular_expense") continue;
        if (t->get_date() < from || t->get_date() > to) continue;
        auto it = account_currencies.find(t->get_account_id());
        if (it != account_currencies.end() && it->second == cur) {
            category_totals[t->get_category_id()] += t->get_amount();
        }
    }

    //формирование итогового списка категорий с суммами
    std::vector<CategoryStat> result;
    result.reserve(categories.size());
    for (const auto& cat : categories) {
        auto it = category_totals.find(cat.id);
        double amount = (it != category_totals.end()) ? it->second : 0.0;
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
    if (n <= 0) return {}; //если нужно вернуть <=0 категорий
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
        if (!t) continue;
        //проверка валидности даты
        if (!t->get_date().ok()) continue;
        //если не совпадает год
        if (static_cast<int>(t->get_date().year()) != year) continue;
        //если не совпадает валюта или счет не найден
        auto acc_cur = get_account_currency(accounts, t->get_account_id());
        if (!acc_cur.has_value() || *acc_cur != cur) continue;
        //получение месяца текущей транзакции
        unsigned month = static_cast<unsigned>(t->get_date().month());
        //проверка диапазона месяца (1-12)
        if (month < 1 || month > 12) continue;
        if (t->get_type() == "income") //если доход
            result[month - 1].month_income += t->get_amount();
        else if (t->get_type() == "expense" || t->get_type() == "regular_expense")
            result[month - 1].month_expense += t->get_amount();
    }
    return result;
}
