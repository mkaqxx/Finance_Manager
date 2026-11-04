#include "report.h"
#include "../utils/chrono_to_string.h"

//статический метод для получения отчета по одной валюте
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
    json j; //создание объекта json
    //создание и заполнение json-полей (доход, расход, баланс)
    j["income"]  = stats.income(transactions, accounts, from, to, cur);
    j["expense"] = stats.expense(transactions, accounts, from, to, cur);
    j["balance"] = stats.balance(accounts, cur);

    //получение топа категорий
    auto top = stats.top_categories(transactions, accounts, categories, from, to, cur, 3);
    //создание json-массива
    json cats = json::array();
    //заполнение json-массива
    for (auto& cs : top) {
        if (cs.amount == 0) continue;
        //создание и заполнение 1 элемента массива
        json c;
        c["name"]   = cs.category.name;
        c["color"]  = cs.category.color;
        c["amount"] = cs.amount;
        //добавление созданного элемента в массив
        cats.push_back(c);
    }
    //запись топа категорий в json-объект
    j["top_categories"] = cats;
    return j;
}


//генерация месячного отчета
json MonthlyReport::generate() const {
    Statistics stats; //объект Statistics для расчета статистики
    json j; //json-объект
    //заполнение json
    j["type"]   = "monthly";
    j["period"] = ym_to_string({from.year(), from.month()});
    //расчет и заполнение для каждой валюты
    j["byn"] = build_currency_section(stats, transactions, accounts, categories, from, to, Currency::BYN, "byn");
    j["usd"] = build_currency_section(stats, transactions, accounts, categories, from, to, Currency::USD, "usd");
    j["eur"] = build_currency_section(stats, transactions, accounts, categories, from, to, Currency::EUR, "eur");
    j["rub"] = build_currency_section(stats, transactions, accounts, categories, from, to, Currency::RUB, "rub");
    return j;
}


//генерация отчета по категориям
json CategoryReport::generate() const {
    Statistics stats; //объект Statistics для расчета статистики
    json j; //json-объект
    //заполнение json
    j["type"] = "category";
    j["from"] = ymd_to_string(from);
    j["to"]   = ymd_to_string(to);
    //цикл по парам валюты "перечисление-строка"
    //каждая итерация - получение информации об одной валюте
    for (auto [cur, name] : std::vector<std::pair<Currency, std::string>>{
        {Currency::BYN, "byn"}, {Currency::USD, "usd"},
        {Currency::EUR, "eur"}, {Currency::RUB, "rub"}})
    {
        //получение расходов по одной валюте
        double total = stats.expense(transactions, accounts, from, to, cur);
        //получение расходов по категориям
        auto cats = stats.expenses_by_category(transactions, accounts, categories, from, to, cur);
        //создание json-массива и его заполнение
        json arr = json::array();
        for (auto& cs : cats) {
            //заполнение информацией об одной категории
            if (cs.amount == 0) continue;
            json c;
            c["name"]    = cs.category.name;
            c["color"]   = cs.category.color;
            c["amount"]  = cs.amount;
            //сколько процентов займет на круговой диаграмме
            //расчет с проверкой на деление на ноль
            c["percent"] = total > 0 ? cs.amount / total * 100.0 : 0.0;
            //добавление в массив
            arr.push_back(c);
        }
        //запись информации в поле данной валюты
        j[name] = arr;
    }
    return j;
}


//генерация годового отчета
json YearlyReport::generate() const {
    Statistics stats; //объект Statistics для расчета статистики
    json j; //json-объект
    //заполнение json
    j["type"] = "yearly";
    j["year"] = year;

    //цикл по парам валюты "перечисление-строка"
    //каждая итерация - получение информации об одной валюте
    for (auto [cur, name] : std::vector<std::pair<Currency, std::string>>{
        {Currency::BYN, "byn"}, {Currency::USD, "usd"},
        {Currency::EUR, "eur"}, {Currency::RUB, "rub"}})
    {
        //получение массива информации о каждом месяце
        auto sums = stats.monthly_summary(transactions, accounts, year, cur);
        //создание json-массива и его заполнение
        json months = json::array();
        for (size_t i = 0; i < sums.size(); i++) {
            //заполнение одного месяца
            json m;
            m["month"]   = i + 1;
            m["income"]  = sums[i].month_income;
            m["expense"] = sums[i].month_expense;
            //добавление элемента в массив
            months.push_back(m);
        }
        //запись в поле одной валюты
        j[name] = months;
    }
    return j;
}