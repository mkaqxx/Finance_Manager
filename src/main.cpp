#include "webview.h"
#include "core/finance_manager.h"
#include <windows.h>


int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    Finance_manager manager;

    webview::webview w(true, nullptr);
    w.set_title("Finance Manager");
    w.set_size(1280, 800, WEBVIEW_HINT_NONE);


    w.bind("getCategories", [&manager](std::string seq) -> std::string {
        json response = json::array();
        for (auto&  category : manager.get_categories()) {
            json c;
            c["id"] = category.id;
            c["name"] = category.name;
            c["type"] = t_type_to_string(category.type);
            c["color"] = category.color;
            response.push_back(c);
        }
        return response.dump();
    });

    // биндинги будут здесь
    w.bind("getMonthlyDashboard", [&manager](std::string seq) -> std::string {
     auto now = std::chrono::system_clock::now();
     std::chrono::year_month_day today{std::chrono::floor<std::chrono::days>(now)};

     // Формируем первый день текущего месяца
     std::chrono::year_month_day first_day{today.year(), today.month(), std::chrono::day{1}};

     // Вызываем генерацию отчета
     json report = manager.get_monthly_report(first_day, today); //[cite: 8, 11]
     return report.dump();
 });


    w.bind("getTransactions", [&manager](std::string seq) -> std::string {
        json response = json::array();
        for (auto * transaction : manager.get_transactions()) {
            json t;
            t["id"] = transaction->get_id();
            t["amount"] = transaction->get_amount();
            t["date"] = ymd_to_string(transaction->get_date());
            t["type"] = transaction->get_type();
            t["category_id"] = transaction->get_category_id();;
            t["account_id"] = transaction->get_account_id();
            if (auto* reg_exp = dynamic_cast<RegularExpense*>(transaction)) {
                t["next_date"] =ymd_to_string(reg_exp->get_next_date());
                t["period"] = Period_to_string(reg_exp->get_period());
            }
            else if (auto* transfer = dynamic_cast<Transfer*>(transaction)) {
                t["destination_id"] = transfer->get_destination_id();
            }
            response.push_back(t);
        }
        return  response.dump();
    });

    w.bind("getAccounts", [&manager](std::string seq) -> std::string {
    json response = json::array();
    for (auto* acc : manager.get_accounts()) { //[cite: 8]
        if (!acc) continue;
        json a;
        a["id"] = acc->get_id();
        a["name"] = acc->get_name();
        a["balance"] = acc->get_balance();
        a["type"] = acc->get_type();
        a["currency"] = CurrencyToString(acc->get_currency());
        if (auto* bank_acc = dynamic_cast<BankAccount*>(acc)) {
            a["number"] = bank_acc->get_last_four_digits();
        }
        else if (auto* sav_acc = dynamic_cast<SavingsAccount*>(acc)) {
            a["goal"] = sav_acc->get_goal_amount();
            a["progress"] = sav_acc->get_progress();
            a["monthly_required"] = sav_acc->monthly_required();
            a["deadline"] = ymd_to_string(sav_acc->get_deadline())
        }

        response.push_back(a);
    }

    return response.dump();
    });


    w.bind("addTransaction", [&manager](std::string seq) -> std::string {
        try {
            json args = json::parse(seq);
            json data = args[0];
            unsigned id = manager.get_transactions().size() + 1;
            double amount = data["amount"];
            auto date = ymd_from_string(data["date"]);
            std::string type = data["type"];
            unsigned account_id = data["account_id"];
            unsigned category_id = 0;

            if (type == "expense" || type == "regular_expense" || type == "transfer") {
            Account* acc = manager.get_account_by_id(account_id); //[cite: 7, 8]
            if (acc->get_balance() < amount) {
                return "{\"error\":\"Недостаточно средств на счете!\"}";
            }
        }
            if (type != "transfer") category_id = data["category_id"];
            if ( type == "income") {
                manager.add_transaction(new Income(id, amount, date, category_id, account_id));
            }
            else if (type == "expense") {
                manager.add_transaction(new Expense (id, amount, date,  category_id, account_id) );
            }
            else if (type == "transfer") {
                unsigned destination_id = data["destination_id"];
                manager.add_transaction(new Transfer(id, amount, date, account_id, destination_id));
            }
            else if (type == "regular_expense") {
                Period p = String_to_period(data["period"]);
                manager.add_transaction(new RegularExpense(id, amount, date, category_id, account_id, date, p));
            }
            return "{\"status\":\"success\"}";
        }
        catch(const std::exception& e) {
            return "{\"error\":\"" + std::string(e.what()) + "\"}";
        }
    });


    w.bind("addAccount", [&manager](std::string seq) -> std::string {
    try {
        json args = json::parse(seq);
        json data = args[0];

        // Генерируем новый ID (размер массива счетов + 1)
        unsigned id = manager.get_accounts().size() + 1;
        std::string name = data["name"];
        double balance = data["balance"];
        Currency currency = StringToCurrency(data["currency"]); //[cite: 1]
        std::string type = data["type"];

        if (type == "cash_account") {
            manager.add_account(new CashAccount(id, name, balance, currency)); //[cite: 2, 7]
        } else if (type == "bank_account") {
            std::string digits = data["last_four_digits"];
            manager.add_account(new BankAccount(id, name, balance, currency, digits)); //[cite: 2, 7]
        } else if (type == "savings_account") {
            double goal = data["goal_amount"];
            std::chrono::year_month_day deadline = ymd_from_string(data["deadline"]);
            manager.add_account(new SavingsAccount(id, name, balance, currency, goal, deadline)); //[cite: 2, 7]
        }

        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
    });


    w.bind("getBudgets", [&manager](std::string seq) -> std::string {
    json response = json::array();
    for (const auto& budget : manager.get_budgets()) { //[cite: 8]
        json b;
        b["category_id"] = budget.get_category().id; //[cite: 4]
        b["name"] = budget.get_category().name; //[cite: 4]
        b["color"] = budget.get_category().color; //[cite: 4]
        b["limit"] = budget.get_limit(); //[cite: 4]
        b["current_amount"] = budget.get_current_amount(); //[cite: 4]
        b["status"] = status_to_string(budget.get_status()); //[cite: 4]
        response.push_back(b);
    }
    return response.dump();
});

    w.bind("addBudget", [&manager](std::string seq) -> std::string {
        try {
            json args = json::parse(seq);
            json data = args[0];

            unsigned cat_id = data["category_id"];
            double limit = data["limit"];

            Category cat = manager.get_category_by_id(cat_id); //[cite: 7, 8]
            manager.add_budget(Budget(cat, limit, 0.0)); //[cite: 4, 7, 8]

            return "{\"status\":\"success\"}";
        } catch (const std::exception& e) {
            return "{\"error\":\"" + std::string(e.what()) + "\"}";
        }
    });



    w.bind("addCategory", [&manager](std::string seq) -> std::string {
    try {
        json args = json::parse(seq);
        json data = args[0];

        unsigned id = manager.get_categories().size() + 1; //[cite: 8]
        std::string name = data["name"];
        std::string type_str = data["type"];
        std::string color = data["color"];

        TransactionType type = t_type_from_string(type_str); //[cite: 3, 4]
        Category new_cat{id, type, name, color}; //[cite: 4]

        manager.add_category(new_cat); //[cite: 7, 8]

        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
});



    w.bind("getCategoryReport", [&manager](std::string seq) -> std::string {
        try {
            json args = json::parse(seq);
            json data = args[0];
            auto from = ymd_from_string(data[0]);
            auto to = ymd_from_string(data[1]);
            json report = manager.get_category_report(from, to);
            return report.dump();
        }
        catch (const std::exception& e) {
            return "{\"error\":\"" + std::string(e.what()) + "\"}";
        }
    });


    w.bind("getYearlyReport", [&manager](std::string seq) -> std::string {
        try {
            json args = json::parse(seq);
            json data = args[0];
            int year = data[0];
            json report = manager.get_yearly_report(year);
            return report.dump();
        }
        catch (const std::exception& e) {
            return "{\"error\":\"" + std::string(e.what()) + "\"}";
        }
    });

    w.navigate("file:///D:/Finance_manager/assets/index.html");
    w.run();
    return 0;
}