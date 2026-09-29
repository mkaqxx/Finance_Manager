#include "report.h"
#include "../utils/chrono_to_string.h"


static json build_currency_section(
    const Statistics& stats,
    const std::vector<Transaction*>& transactions,
    const std::vector<Account*>& accounts,
    const std::vector<Category>& categories,
    std::chrono::year_month_day from,
    std::chrono::year_month_day to,
    Currency cur,
    const std::string& cur_name)
{
    json j;
    j["income"]  = stats.income(transactions, accounts, from, to, cur);
    j["expense"] = stats.expense(transactions, accounts, from, to, cur);
    j["balance"] = stats.balance(accounts, cur);

    auto top = stats.top_categories(transactions, accounts, categories, from, to, cur, 3);
    json cats = json::array();
    for (auto& cs : top) {
        if (cs.amount == 0) continue;
        json c;
        c["name"]   = cs.category.name;
        c["color"]  = cs.category.color;
        c["amount"] = cs.amount;
        cats.push_back(c);
    }
    j["top_categories"] = cats;
    return j;
}


json MonthlyReport::generate() const {
    Statistics stats;
    json j;
    j["type"]   = "monthly";
    j["period"] = ym_to_string({from.year(), from.month()});
    j["byn"] = build_currency_section(stats, transactions, accounts, categories, from, to, Currency::BYN, "byn");
    j["usd"] = build_currency_section(stats, transactions, accounts, categories, from, to, Currency::USD, "usd");
    j["eur"] = build_currency_section(stats, transactions, accounts, categories, from, to, Currency::EUR, "eur");
    j["rub"] = build_currency_section(stats, transactions, accounts, categories, from, to, Currency::RUB, "rub");
    return j;
}

json CategoryReport::generate() const {
    Statistics stats;
    json j;
    j["type"] = "category";
    j["from"] = ymd_to_string(from);
    j["to"]   = ymd_to_string(to);

    for (auto [cur, name] : std::vector<std::pair<Currency, std::string>>{
        {Currency::BYN, "byn"}, {Currency::USD, "usd"},
        {Currency::EUR, "eur"}, {Currency::RUB, "rub"}})
    {
        double total = stats.expense(transactions, accounts, from, to, cur);
        auto cats = stats.expenses_by_category(transactions, accounts, categories, from, to, cur);
        json arr = json::array();
        for (auto& cs : cats) {
            if (cs.amount == 0) continue;
            json c;
            c["name"]    = cs.category.name;
            c["color"]   = cs.category.color;
            c["amount"]  = cs.amount;
            c["percent"] = total > 0 ? cs.amount / total * 100.0 : 0.0;
            arr.push_back(c);
        }
        j[name] = arr;
    }
    return j;
}


json YearlyReport::generate() const {
    Statistics stats;
    json j;
    j["type"] = "yearly";
    j["year"] = year;

    for (auto [cur, name] : std::vector<std::pair<Currency, std::string>>{
        {Currency::BYN, "byn"}, {Currency::USD, "usd"},
        {Currency::EUR, "eur"}, {Currency::RUB, "rub"}})
    {
        auto sums = stats.monthly_summary(transactions, accounts, year, cur);
        json months = json::array();
        for (size_t i = 0; i < sums.size(); i++) {
            json m;
            m["month"]   = i + 1;
            m["income"]  = sums[i].month_income;
            m["expense"] = sums[i].month_expense;
            months.push_back(m);
        }
        j[name] = months;
    }
    return j;
}