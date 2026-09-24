#pragma once
#include "finance_manager.h"
#include "webview.h"


class ApiHandler {
    Finance_manager& manager;
    webview::webview& w;
    public:
    ApiHandler(Finance_manager& manager, webview::webview& w) : manager(manager), w(w) {}
    void register_all();
    private:
    //получение
    std::string handle_get_accounts(std::string seq);
    std::string handle_get_transactions(std::string seq);
    std::string handle_get_categories(std::string seq);
    std::string handle_get_budgets(std::string seq);
    std::string handle_get_transactions_by_account(std::string seq);
    std::string handle_get_monthly_dashboard(std::string seq);
    std::string handle_get_category_report(std::string seq);
    std::string handle_get_yearly_report(std::string seq);
    //добавление
    std::string handle_add_account(std::string seq);
    std::string handle_add_transaction(std::string seq);
    std::string handle_add_category(std::string seq);
    std::string handle_add_budget(std::string seq);
    //удаление
    std::string handle_remove_account(std::string seq);
    std::string handle_remove_transaction(std::string seq);
    std::string handle_remove_category(std::string seq);
    std::string handle_remove_budget(std::string seq);
    //редактирование
    std::string handle_edit_account(std::string seq);
    std::string handle_edit_budget(std::string seq);
};