#include "statistics.h"


Currency Statistics::get_account_currency( const std::vector<Account*>& accounts, unsigned account_id) const{
    for (const auto* acc : accounts)
        if (acc->get_id() == account_id)
            return acc->get_currency();
    return Currency::BYN; // fallback
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
    std::vector<CategoryStat> result;
    for (auto& cat : categories) {
        double amount = 0;
        for (auto* t : transactions) {
            if (t->get_category_id() == cat.id
                && t->get_date() >= from
                && t->get_date() <= to
                && (t->get_type() == "expense" || t->get_type() == "regular_expense")
                && get_account_currency(accounts, t->get_account_id()) == cur)
                amount += t->get_amount();
        }
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
    auto result = expenses_by_category(transactions, accounts, categories, from, to, cur);
    int size = result.size();
    for (int i = 0; i < size; i++) {
        int max = i;
        for (int j = i + 1; j < size; j++)
            if (result[j].amount > result[max].amount)
                max = j;
        auto temp = result[i];
        result[i] = result[max];
        result[max] = temp;
    }
    if (n < size) result.resize(n);
    return result;
}



std::vector<Monthly_sum> Statistics::monthly_summary(
    const std::vector<Transaction*>& transactions,
    const std::vector<Account*>& accounts,
    int year,
    Currency cur) const
{
    std::vector<Monthly_sum> result(12);
    for (auto* t : transactions) {
        if (static_cast<int>(t->get_date().year()) != year) continue;
        if (get_account_currency(accounts, t->get_account_id()) != cur) continue;
        unsigned month = static_cast<unsigned>(t->get_date().month());
        if (t->get_type() == "income")
            result[month - 1].month_income += t->get_amount();
        else if (t->get_type() == "expense" || t->get_type() == "regular_expense")
            result[month - 1].month_expense += t->get_amount();
    }
    return result;
}
