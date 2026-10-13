#include "api_handler.h"
#include <cmath>

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
    try {
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
                double req = sav_acc->monthly_required();
                a["monthly_required"] = req > 0.0 ? static_cast<int>(req) + 1 : 0;
                a["deadline"] = ymd_to_string(sav_acc->get_deadline());
            }

            response.push_back(a);
        }
        return response.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::get_transactions() {
    try {
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
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::get_categories() {
    try {
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
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::get_budgets() {
    try {
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
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::get_transactions_by_account(unsigned id) {
    try {
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
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::get_monthly_dashboard() {
    try {
        auto now = std::chrono::system_clock::now();
        std::chrono::year_month_day today{std::chrono::floor<std::chrono::days>(now)};
        std::chrono::year_month_day first_day{today.year(), today.month(), std::chrono::day{1}};

        json report = mgr().get_monthly_report(first_day, today);
        return report.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::get_category_report(const std::string& from, const std::string& to) {
    try {
        auto from_date = ymd_from_string(from);
        auto to_date = ymd_from_string(to);
        json report = mgr().get_category_report(from_date, to_date);
        return report.dump();
    }
    catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::get_yearly_report(int year) {
    try {
        json report = mgr().get_yearly_report(year);
        return report.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
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
        std::string name = data.value("name", "");
        if (name.empty()) {
            return json{{"error", "Название счёта не может быть пустым"}}.dump();
        }
        double balance = data.value("balance", 0.0);
        if (std::isnan(balance) || std::isinf(balance) || balance < 0.0) {
            return json{{"error", "Баланс счёта должен быть неотрицательным числом"}}.dump();
        }
        Currency currency = StringToCurrency(data.value("currency", "byn"));
        std::string type = data.value("type", "cash_account");

        std::expected<void, AppError> res;
        if (type == "cash_account") {
            res = mgr().add_account(std::make_unique<CashAccount>(id, name, balance, currency));
        } else if (type == "bank_account") {
            std::string digits = data.value("last_four_digits", "");
            res = mgr().add_account(std::make_unique<BankAccount>(id, name, balance, currency, digits));
        } else if (type == "savings_account") {
            double goal = data.value("goal_amount", 0.0);
            if (std::isnan(goal) || std::isinf(goal) || goal <= 0.0) {
                return json{{"error", "Целевая сумма накопления должна быть больше нуля"}}.dump();
            }
            std::chrono::year_month_day deadline = ymd_from_string(data.value("deadline", "2099-01-01"));
            res = mgr().add_account(std::make_unique<SavingsAccount>(id, name, balance, currency, goal, deadline));
        } else {
            return json{{"error", "Неизвестный тип счёта"}}.dump();
        }

        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }

        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
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
        double amount = data.value("amount", 0.0);
        if (std::isnan(amount) || std::isinf(amount) || amount <= 0.0) {
            return json{{"error", "Сумма операции должна быть больше нуля"}}.dump();
        }

        auto date = ymd_from_string(data.value("date", "2000-01-01"));
        std::string type = data.value("type", "expense");
        unsigned account_id = data.value("account_id", 0u);
        unsigned category_id = 0;

        if (type == "expense" || type == "regular_expense" || type == "transfer") {
            Account* acc = mgr().get_account_by_id(account_id);
            if (!acc) {
                return json{{"error", "Счёт списания не найден"}}.dump();
            }
            if (acc->get_balance() < amount) {
                return json{{"error", "Недостаточно средств на счёте"}}.dump();
            }
        }
        if (type != "transfer") category_id = data.value("category_id", 0u);
        std::expected<void, AppError> res;
        if (type == "income") {
            res = mgr().add_transaction(std::make_unique<Income>(id, amount, date, category_id, account_id));
        }
        else if (type == "expense") {
            res = mgr().add_transaction(std::make_unique<Expense>(id, amount, date, category_id, account_id));
        }
        else if (type == "transfer") {
            unsigned destination_id = data.value("destination_id", 0u);
            if (account_id == destination_id) {
                return json{{"error", "Счёт списания и счёт зачисления не могут совпадать"}}.dump();
            }
            Account* src = mgr().get_account_by_id(account_id);
            Account* dest = mgr().get_account_by_id(destination_id);
            if (src && dest) {
                if (src->get_currency() != dest->get_currency()) {
                    return json{{"error", "Нельзя переводить между счетами с разными валютами"}}.dump();
                }
                res = mgr().add_transaction(std::make_unique<Transfer>(id, amount, date, account_id, destination_id));
            }
            else {
                return json{{"error", "Один из счетов для перевода не найден"}}.dump();
            }
        }
        else if (type == "regular_expense") {
            Period p = String_to_period(data.value("period", "monthly"));
            auto reg = std::make_unique<RegularExpense>(id, amount, date, category_id, account_id, date, p);
            reg->update_next_date(); // Первое списание сейчас, следующее - через период
            res = mgr().add_transaction(std::move(reg));
        } else {
            return json{{"error", "Неизвестный тип транзакции"}}.dump();
        }

        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    }
    catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
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
        std::string name = data.value("name", "");
        if (name.empty()) {
            return json{{"error", "Название категории не может быть пустым"}}.dump();
        }
        std::string type_str = data.value("type", "expense");
        std::string color = data.value("color", "#00ff66");

        TransactionType type = t_type_from_string(type_str);
        Category new_cat{id, type, name, color};

        auto res = mgr().add_category(new_cat);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }

        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::add_budget(const std::string& data_json) {
    try {
        json data = json::parse(data_json);
        if (data.is_array() && !data.empty()) {
            data = data[0];
        }

        unsigned cat_id = data.value("category_id", 0u);
        double limit = data.value("limit", 0.0);
        if (std::isnan(limit) || std::isinf(limit) || limit <= 0.0) {
            return json{{"error", "Лимит бюджета должен быть положительным числом"}}.dump();
        }
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
            return json{{"error", res.error().message}}.dump();
        }

        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::remove_account(unsigned id) {
    try {
        auto res = mgr().remove_account(id);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::remove_transaction(unsigned id) {
    try {
        auto res = mgr().remove_transaction(id);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::remove_category(unsigned id) {
    try {
        auto res = mgr().remove_category(id);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::remove_budget(unsigned id) {
    try {
        auto res = mgr().remove_budget(id);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::edit_account(const std::string& data_json) {
    try {
        json data = json::parse(data_json);
        if (data.is_array() && !data.empty()) {
            data = data[0];
        }

        unsigned id = data.value("id", 0u);
        std::string name = data.value("name", "");
        if (name.empty()) {
            return json{{"error", "Название счёта не может быть пустым"}}.dump();
        }
        double balance = data.value("balance", 0.0);
        if (std::isnan(balance) || std::isinf(balance) || balance < 0.0) {
            return json{{"error", "Баланс счёта должен быть неотрицательным числом"}}.dump();
        }
        Currency currency = StringToCurrency(data.value("currency", "byn"));
        std::string last_four_digits = data.value("last_four_digits", "");
        double goal_amount = data.value("goal_amount", 0.0);
        std::chrono::year_month_day deadline = ymd_from_string(data.value("deadline", "2099-01-01"));

        auto res = mgr().edit_account(id, name, balance, currency, last_four_digits, goal_amount, deadline);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    }
    catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

std::string ApiHandler::edit_budget(const std::string& data_json) {
    try {
        json data = json::parse(data_json);
        if (data.is_array() && !data.empty()) {
            data = data[0];
        }

        unsigned cat_id = data.value("category_id", 0u);
        double limit = data.value("limit", 0.0);
        if (std::isnan(limit) || std::isinf(limit) || limit <= 0.0) {
            return json{{"error", "Лимит бюджета должен быть положительным числом"}}.dump();
        }

        auto res = mgr().edit_budget(cat_id, limit);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}

#ifdef __EMSCRIPTEN__
static bool fm_save_data(Finance_manager& self) { return self.save_data().has_value(); }
static bool fm_remove_account(Finance_manager& self, unsigned id) { return self.remove_account(id).has_value(); }
static bool fm_remove_transaction(Finance_manager& self, unsigned id) { return self.remove_transaction(id).has_value(); }
static bool fm_remove_category(Finance_manager& self, unsigned id) { return self.remove_category(id).has_value(); }
static bool fm_remove_budget(Finance_manager& self, unsigned id) { return self.remove_budget(id).has_value(); }
static bool fm_edit_budget(Finance_manager& self, unsigned id, double l) { return self.edit_budget(id, l).has_value(); }

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
        .function("saveData", &fm_save_data)
        .function("getCategories", &Finance_manager::get_categories)
        .function("getBudgets", &Finance_manager::get_budgets)
        .function("removeAccount", &fm_remove_account)
        .function("removeTransaction", &fm_remove_transaction)
        .function("removeCategory", &fm_remove_category)
        .function("removeBudget", &fm_remove_budget)
        .function("editBudget", &fm_edit_budget);

    // Глобальная функция-геттер для быстрого доступа из JS
    function("getApiHandler", &get_wasm_api_handler, allow_raw_pointers());
}
#endif
