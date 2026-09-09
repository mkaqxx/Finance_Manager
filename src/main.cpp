#include "webview.h"
#include "core/finance_manager.h"
#include <windows.h>


int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    Finance_manager manager;

    webview::webview w(true, nullptr);
    w.set_title("Finance Manager");
    w.set_size(1280, 800, WEBVIEW_HINT_NONE);

    // биндинги будут здесь
    w.bind("getFinanceData", [&manager](std::string seq) -> std::string {
        json response;
        response["balance"] = manager.get_balance();
        response["expenses"] = manager.get_expenses_for_month();
        response["incomes"] = manager.get_incomes_for_month();
        return response.dump();
    });


    w.bind("getTransactions", [&manager](std::string seq) -> std::string {
        json response = json::array();
        for (auto * transaction : manager.get_transactions()) {
            json t;
            t["id"] = transaction->get_id();
            t["amount"] = transaction->get_amount();
            t["date"] = ymd_to_string(transaction->get_date());
            t["type"] = transaction->get_type();
            auto category = manager.get_category_by_id(transaction->get_category_id());
            t["category"] = category.name;;
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
    w.navigate("file:///D:/Finance_manager/assets/index.html");
    w.run();
    return 0;
}