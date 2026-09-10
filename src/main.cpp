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
        }

        response.push_back(a);
    }

    return response.dump();
});


    w.navigate("file:///D:/Finance_manager/assets/index.html");
    w.run();
    return 0;
}