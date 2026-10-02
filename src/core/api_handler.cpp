#include "api_handler.h"

static ApiHandler* s_global_api = nullptr;

ApiHandler::ApiHandler()
    : owned_manager(std::make_unique<Finance_manager>()),
      manager_ptr(owned_manager.get()) {
    if (!s_global_api) {
        s_global_api = this;
    }
}

ApiHandler::ApiHandler(Finance_manager& manager)
    : owned_manager(nullptr),
      manager_ptr(&manager) {
    if (!s_global_api) {
        s_global_api = this;
    }
}

void ApiHandler::init() {
    owned_manager = std::make_unique<Finance_manager>();
    manager_ptr = owned_manager.get();
}

ApiHandler* get_wasm_api_handler() {
    static ApiHandler s_fallback_api;
    return s_global_api ? s_global_api : &s_fallback_api;
}

std::string ApiHandler::get_accounts() {
    json response = json::array();
    for (auto* acc : mgr().get_accounts_raw()) {
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
            a["monthly_required"] = static_cast<int>(sav_acc->monthly_required()) + 1;
            a["deadline"] = ymd_to_string(sav_acc->get_deadline());
        }

        response.push_back(a);
    }

    return response.dump();
}

std::string ApiHandler::get_transactions() {
    json response = json::array();
    for (auto* transaction : mgr().get_transactions_raw()) {
        if (!transaction) continue;
        json t;
        t["id"] = transaction->get_id();
        t["amount"] = transaction->get_amount();
        t["date"] = ymd_to_string(transaction->get_date());
        t["type"] = transaction->get_type();
        t["category_id"] = transaction->get_category_id();
        t["account_id"] = transaction->get_account_id();
        Account* acc = mgr().get_account_by_id(transaction->get_account_id());
        if (!acc) continue;
        t["currency"] = CurrencyToString(acc->get_currency());
        if (auto* reg_exp = dynamic_cast<RegularExpense*>(transaction)) {
            t["next_date"] = ymd_to_string(reg_exp->get_next_date());
            t["period"] = Period_to_string(reg_exp->get_period());
        }
        else if (auto* transfer = dynamic_cast<Transfer*>(transaction)) {
            t["destination_id"] = transfer->get_destination_id();
        }
        response.push_back(t);
    }
    return response.dump();
}

std::string ApiHandler::get_categories() {
    json response = json::array();
    for (const auto& category : mgr().get_categories()) {
        json c;
        c["id"] = category.id;
        c["name"] = category.name;
        c["type"] = t_type_to_string(category.type);
        c["color"] = category.color;
        response.push_back(c);
    }
    return response.dump();
}

std::string ApiHandler::get_budgets() {
    json response = json::array();
    for (const auto& budget : mgr().get_budgets()) {
        json b;
        b["category_id"] = budget.get_category().id;
        b["name"] = budget.get_category().name;
        b["color"] = budget.get_category().color;
        b["limit"] = budget.get_limit();
        b["current_amount"] = budget.get_current_amount();
        b["status"] = status_to_string(budget.get_status());
        response.push_back(b);
    }
    return response.dump();
}

std::string ApiHandler::get_transactions_by_account(unsigned id) {
    json response = json::array();
    for (auto* transaction : mgr().get_transactions_raw()) {
        if (!transaction) continue;
        if (transaction->get_account_id() == id) {
            json t;
            t["id"] = transaction->get_id();
            t["amount"] = transaction->get_amount();
            t["date"] = ymd_to_string(transaction->get_date());
            t["type"] = transaction->get_type();
            t["category_id"] = transaction->get_category_id();
            t["account_id"] = transaction->get_account_id();
            Account* acc = mgr().get_account_by_id(transaction->get_account_id());
            if (!acc) continue;
            t["currency"] = CurrencyToString(acc->get_currency());
            if (auto* reg_exp = dynamic_cast<RegularExpense*>(transaction)) {
                t["next_date"] = ymd_to_string(reg_exp->get_next_date());
                t["period"] = Period_to_string(reg_exp->get_period());
            }
            else if (auto* transfer = dynamic_cast<Transfer*>(transaction)) {
                t["destination_id"] = transfer->get_destination_id();
            }
            response.push_back(t);
        }
    }
    return response.dump();
}

std::string ApiHandler::get_monthly_dashboard() {
    auto now = std::chrono::system_clock::now();
    std::chrono::year_month_day today{std::chrono::floor<std::chrono::days>(now)};
    std::chrono::year_month_day first_day{today.year(), today.month(), std::chrono::day{1}};

    json report = mgr().get_monthly_report(first_day, today);
    return report.dump();
}

std::string ApiHandler::get_category_report(const std::string& from, const std::string& to) {
    try {
        auto from_date = ymd_from_string(from);
        auto to_date = ymd_from_string(to);
        json report = mgr().get_category_report(from_date, to_date);
        return report.dump();
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::string ApiHandler::get_yearly_report(int year) {
    json report = mgr().get_yearly_report(year);
    return report.dump();
}

std::string ApiHandler::add_account(const std::string& data_json) {
    try {
        json data = json::parse(data_json);
        if (data.is_array() && !data.empty()) {
            data = data[0];
        }

        unsigned max_id = 0;
        for (const auto* a : mgr().get_accounts_raw()) {
            if (a && a->get_id() > max_id) max_id = a->get_id();
        }
        unsigned id = max_id + 1;
        std::string name = data["name"];
        double balance = data["balance"];
        Currency currency = StringToCurrency(data["currency"]);
        std::string type = data["type"];

        std::expected<void, int> res;
        if (type == "cash_account") {
            res = mgr().add_account(std::make_unique<CashAccount>(id, name, balance, currency));
        } else if (type == "bank_account") {
            std::string digits = data["last_four_digits"];
            res = mgr().add_account(std::make_unique<BankAccount>(id, name, balance, currency, digits));
        } else if (type == "savings_account") {
            double goal = data["goal_amount"];
            std::chrono::year_month_day deadline = ymd_from_string(data["deadline"]);
            res = mgr().add_account(std::make_unique<SavingsAccount>(id, name, balance, currency, goal, deadline));
        } else {
            return "{\"error\":\"Неизвестный тип счёта\"}";
        }

        if (!res) {
            return "{\"error\":\"Ошибка сохранения счёта (код: " + std::to_string(res.error()) + ")\"}";
        }

        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::string ApiHandler::add_transaction(const std::string& data_json) {
    try {
        json data = json::parse(data_json);
        if (data.is_array() && !data.empty()) {
            data = data[0];
        }

        unsigned max_id = 0;
        for (const auto* t : mgr().get_transactions_raw()) {
            if (t && t->get_id() > max_id) max_id = t->get_id();
        }
        unsigned id = max_id + 1;
        double amount = data["amount"];
        auto date = ymd_from_string(data["date"]);
        std::string type = data["type"];
        unsigned account_id = data["account_id"];
        unsigned category_id = 0;

        if (type == "expense" || type == "regular_expense" || type == "transfer") {
            Account* acc = mgr().get_account_by_id(account_id);
            if (!acc) {
                return "{\"error\":\"Счёт списания не найден!\"}";
            }
            if (acc->get_balance() < amount) {
                return "{\"error\":\"Недостаточно средств на счете!\"}";
            }
        }
        if (type != "transfer") category_id = data["category_id"];
        std::expected<void, int> res;
        if (type == "income") {
            res = mgr().add_transaction(std::make_unique<Income>(id, amount, date, category_id, account_id));
        }
        else if (type == "expense") {
            res = mgr().add_transaction(std::make_unique<Expense>(id, amount, date, category_id, account_id));
        }
        else if (type == "transfer") {
            unsigned destination_id = data["destination_id"];
            Account* src = mgr().get_account_by_id(account_id);
            Account* dest = mgr().get_account_by_id(destination_id);
            if (src && dest) {
                if (src->get_currency() != dest->get_currency()) {
                    return "{\"error\":\"Нельзя переводить между счетами с разными валютами\"}";
                }
                res = mgr().add_transaction(std::make_unique<Transfer>(id, amount, date, account_id, destination_id));
            }
            else return "{\"error\":\"аккаунт не найден или поврежден\"}";
        }
        else if (type == "regular_expense") {
            Period p = String_to_period(data["period"]);
            res = mgr().add_transaction(std::make_unique<RegularExpense>(id, amount, date, category_id, account_id, date, p));
        } else {
            return "{\"error\":\"Неизвестный тип транзакции\"}";
        }

        if (!res) {
            return "{\"error\":\"Ошибка сохранения транзакции (код: " + std::to_string(res.error()) + ")\"}";
        }
        return "{\"status\":\"success\"}";
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::string ApiHandler::add_category(const std::string& data_json) {
    try {
        json data = json::parse(data_json);
        if (data.is_array() && !data.empty()) {
            data = data[0];
        }

        unsigned max_id = 0;
        for (const auto& c : mgr().get_categories()) {
            if (c.id > max_id) max_id = c.id;
        }
        unsigned id = max_id + 1;
        std::string name = data["name"];
        std::string type_str = data["type"];
        std::string color = data["color"];

        TransactionType type = t_type_from_string(type_str);
        Category new_cat{id, type, name, color};

        auto res = mgr().add_category(new_cat);
        if (!res) {
            return "{\"error\":\"Ошибка добавления категории (код: " + std::to_string(res.error()) + ")\"}";
        }

        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::string ApiHandler::add_budget(const std::string& data_json) {
    try {
        json data = json::parse(data_json);
        if (data.is_array() && !data.empty()) {
            data = data[0];
        }

        unsigned cat_id = data["category_id"];
        double limit = data["limit"];
        double current_amount = 0.0;
        for (const auto* t: mgr().get_transactions_raw()) {
            if (t && t->get_category_id() == cat_id && (t->get_type() == "expense" || t->get_type() == "regular_expense")) {
                Account* acc = mgr().get_account_by_id(t->get_account_id());
                // Лимиты считаются только для базовой валюты (BYN)
                if (acc && acc->get_currency() == Currency::BYN) {
                    current_amount += t->get_amount();
                }
            }
        }
        Category cat = mgr().get_category_by_id(cat_id);
        auto res = mgr().add_budget(Budget(cat, limit, current_amount));
        if (!res) {
            return "{\"error\":\"Ошибка добавления бюджета (код: " + std::to_string(res.error()) + ")\"}";
        }

        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::string ApiHandler::remove_account(unsigned id) {
    try {
        auto res = mgr().remove_account(id);
        if (!res) {
            return "{\"error\":\"Счёт не найден или ошибка сохранения (код: " + std::to_string(res.error()) + ")\"}";
        }
        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::string ApiHandler::remove_transaction(unsigned id) {
    auto res = mgr().remove_transaction(id);
    if (!res) {
        return "{\"error\":\"Ошибка удаления транзакции (код: " + std::to_string(res.error()) + ")\"}";
    }
    return "{\"status\":\"success\"}";
}

std::string ApiHandler::remove_category(unsigned id) {
    auto res = mgr().remove_category(id);
    if (!res) {
        return "{\"error\":\"Ошибка удаления категории (код: " + std::to_string(res.error()) + ")\"}";
    }
    return "{\"status\":\"success\"}";
}

std::string ApiHandler::remove_budget(unsigned id) {
    auto res = mgr().remove_budget(id);
    if (!res) {
        return "{\"error\":\"Ошибка удаления бюджета (код: " + std::to_string(res.error()) + ")\"}";
    }
    return "{\"status\":\"success\"}";
}

std::string ApiHandler::edit_account(const std::string& data_json) {
    try {
        json data = json::parse(data_json);
        if (data.is_array() && !data.empty()) {
            data = data[0];
        }

        Account* edit_acc = mgr().get_account_by_id(data["id"]);
        if (edit_acc) {
            edit_acc->set_name(data["name"]);
            edit_acc->set_balance(data["balance"]);
            edit_acc->set_currency(StringToCurrency(data["currency"]));
            if (data["type"] == "bank_account") {
                if (BankAccount* edit_bank = dynamic_cast<BankAccount*>(edit_acc))
                    edit_bank->set_last_four_digits(data["last_four_digits"]);
            }
            else if (data["type"] == "savings_account") {
                if (SavingsAccount* edit_savings = dynamic_cast<SavingsAccount*>(edit_acc)) {
                    edit_savings->set_goal_amount(data["goal_amount"]);
                    edit_savings->set_deadline(ymd_from_string(data["deadline"]));
                }
            }
        }
        auto res = mgr().save_data();
        if (!res) {
            return "{\"error\":\"Ошибка сохранения счёта (код: " + std::to_string(res.error()) + ")\"}";
        }
        return "{\"status\":\"success\"}";
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

std::string ApiHandler::edit_budget(const std::string& data_json) {
    try {
        json data = json::parse(data_json);
        if (data.is_array() && !data.empty()) {
            data = data[0];
        }

        auto res = mgr().edit_budget(data["category_id"], data["limit"]);
        if (!res) {
            return "{\"error\":\"Ошибка обновления бюджета (код: " + std::to_string(res.error()) + ")\"}";
        }
        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(finance_wasm_module) {
    using namespace emscripten;

    // Регистрация коллекций STL
    register_vector<std::string>("VectorString");
    register_vector<Category>("VectorCategory");

    // Регистрация Enum-типов
    enum_<Currency>("Currency")
        .value("BYN", Currency::BYN)
        .value("USD", Currency::USD)
        .value("EUR", Currency::EUR)
        .value("RUB", Currency::RUB);

    enum_<TransactionType>("TransactionType")
        .value("INCOME", TransactionType::INCOME)
        .value("EXPENSE", TransactionType::EXPENSE);

    // Регистрация Category как value_object для прямого взаимодействия с JS
    value_object<Category>("Category")
        .field("id", &Category::id)
        .field("type", &Category::type)
        .field("name", &Category::name)
        .field("color", &Category::color);

    // Регистрация моста ApiHandler
    class_<ApiHandler>("ApiHandler")
        .constructor<>()
        .function("init", &ApiHandler::init)
        .function("getAccounts", &ApiHandler::get_accounts)
        .function("getTransactions", &ApiHandler::get_transactions)
        .function("getCategories", &ApiHandler::get_categories)
        .function("getBudgets", &ApiHandler::get_budgets)
        .function("getTransactionsByAccount", &ApiHandler::get_transactions_by_account)
        .function("getMonthlyDashboard", &ApiHandler::get_monthly_dashboard)
        .function("getCategoryReport", &ApiHandler::get_category_report)
        .function("getYearlyReport", &ApiHandler::get_yearly_report)
        .function("addAccount", &ApiHandler::add_account)
        .function("addTransaction", &ApiHandler::add_transaction)
        .function("addCategory", &ApiHandler::add_category)
        .function("addBudget", &ApiHandler::add_budget)
        .function("removeAccount", &ApiHandler::remove_account)
        .function("removeTransaction", &ApiHandler::remove_transaction)
        .function("removeCategory", &ApiHandler::remove_category)
        .function("removeBudget", &ApiHandler::remove_budget)
        .function("editAccount", &ApiHandler::edit_account)
        .function("editBudget", &ApiHandler::edit_budget)
        .function("getFinanceManager", &ApiHandler::get_finance_manager, return_value_policy::reference());

    // Регистрация корневого класса FinanceManager
    class_<Finance_manager>("FinanceManager")
        .constructor<>()
        .function("saveData", &Finance_manager::save_data)
        .function("getCategories", &Finance_manager::get_categories)
        .function("getBudgets", &Finance_manager::get_budgets)
        .function("removeAccount", &Finance_manager::remove_account)
        .function("removeTransaction", &Finance_manager::remove_transaction)
        .function("removeCategory", &Finance_manager::remove_category)
        .function("removeBudget", &Finance_manager::remove_budget)
        .function("editBudget", &Finance_manager::edit_budget);

    // Глобальная функция-геттер для быстрого доступа из JS
    function("getApiHandler", &get_wasm_api_handler, allow_raw_pointers());
}
#endif
