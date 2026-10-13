#include "storage.h"


void Storage::save_accounts(json& j, const std::vector<std::unique_ptr<Account>> &accounts) {
    json accs = json::array();
    for (const auto& acc : accounts) {
        json a;
        a["id"] = acc->get_id();
        a["name"] = acc->get_name();
        a["balance"] = acc->get_balance();
        a["type"] = acc->get_type();
        a["currency"] = CurrencyToString(acc->get_currency());
        if (auto* bank_acc = dynamic_cast<BankAccount*>(acc.get())) {
            a["last_four_digits"] = bank_acc->get_last_four_digits();
        }
        else if (auto* save_acc = dynamic_cast<SavingsAccount*>(acc.get())) {
            a["goal_amount"] = save_acc->get_goal_amount();
            a["deadline"] = ymd_to_string(save_acc->get_deadline());
        }

        accs.push_back(a);
    }
    j["accounts"] = accs;

}


void Storage::save_transactions(json& j, const std::vector<std::unique_ptr<Transaction>> &transactions) {
    json trans = json::array();
    for (const auto& tran : transactions) {
        json t;
        t["id"] = tran->get_id();
        t["amount"] = tran->get_amount();
        t["date"] = ymd_to_string(tran->get_date());
        t["type"] = tran->get_type();
        t["category_id"] = tran->get_category_id();
        t["account_id"] = tran->get_account_id();
        if (auto* reg_exp = dynamic_cast<RegularExpense*>(tran.get())) {
            t["next_date"] =ymd_to_string(reg_exp->get_next_date());
            t["period"] = Period_to_string(reg_exp->get_period());
        }
        else if (auto* transfer = dynamic_cast<Transfer*>(tran.get())) {
            t["destination_id"] = transfer->get_destination_id();
        }
        trans.push_back(t);
    }
    j["transactions"] = trans;

}


void Storage::save_categories(json& j, const std::vector<Category> &categories) {
    json cats = json::array();
    for (Category cat : categories) {
        json c;
        c["id"] = cat.id;
        c["transaction_type"] = t_type_to_string(cat.type);
        c["name"] = cat.name;
        c["color"] = cat.color;
        cats.push_back(c);
    }
    j["categories"] = cats;
}


void Storage::save_budgets(json& j, const std::vector<Budget> &budgets) {
    json buds = json::array();
    for (Budget bud : budgets) {
        json b;
        b["category_id"] = bud.get_category().id;
        b["limit"] = bud.get_limit();
        b["current_amount"] = bud.get_current_amount();
        buds.push_back(b);
    }
    j["budgets"] = buds;
}

#ifdef _WIN32
#include <windows.h>
#endif

std::expected<void, AppError> Storage::save(const std::vector<std::unique_ptr<Account>>& accounts,
                   const std::vector<std::unique_ptr<Transaction>>& transactions,
                   const std::vector<Category>& categories,
                   const std::vector<Budget>& budgets) {
    json j;
    save_accounts(j, accounts);
    save_transactions(j, transactions);
    save_categories(j, categories);
    save_budgets(j, budgets);

    std::string temp_name = filename + ".tmp";
    auto res = open_out_file(temp_name);
    if (!res) {
       return std::unexpected(res.error());
    }
    std::ofstream outfile = std::move(res.value());

    outfile << j.dump(4);
    outfile.flush();
    outfile.close();

#ifdef _WIN32
    std::wstring wtemp(temp_name.begin(), temp_name.end());
    std::wstring wfinal(filename.begin(), filename.end());
    if (!MoveFileExW(wtemp.c_str(), wfinal.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        // Fallback to std::filesystem in case of failure
        std::error_code ec;
        if (std::filesystem::exists(filename)) {
            std::filesystem::remove(filename, ec);
        }
        std::filesystem::rename(temp_name, filename, ec);
        if (ec) {
            return std::unexpected(AppError{ErrorCode::IoError, "Не удалось заменить файл базы данных: " + ec.message()});
        }
    }
#else
    std::error_code ec;
    std::filesystem::rename(temp_name, filename, ec);
    if (ec) {
        return std::unexpected(AppError{ErrorCode::IoError, "Не удалось заменить файл базы данных: " + ec.message()});
    }
#endif
    return {};
}

std::expected<void, AppError> Storage::load(std::vector<std::unique_ptr<Account>>& accounts,
                   std::vector<std::unique_ptr<Transaction>>& transactions,
                   std::vector<Category>& categories,
                   std::vector<Budget>& budgets) {

    // Восстановление, если питание отключилось во время предыдущего сохранения
    if (!std::filesystem::exists(filename)) {
        std::string temp_name = filename + ".tmp";
        if (std::filesystem::exists(temp_name)) {
            std::error_code ec;
            std::filesystem::rename(temp_name, filename, ec);
        }
        if (!std::filesystem::exists(filename)) {
            return {};
        }
    }
    
    auto res = open_in_file(filename);
    if (!res) {
        return std::unexpected(res.error());
    }
    std::ifstream file = std::move(res.value());
    
    json j;
    try {
        j = json::parse(file);
    } catch (const json::parse_error& e) {
        file.close();
        return std::unexpected(AppError{ErrorCode::ParseError, "Ошибка чтения data.json: неверный JSON синтаксис (" + std::string(e.what()) + ")"});
    } catch (const std::exception& e) {
        file.close();
        return std::unexpected(AppError{ErrorCode::ParseError, "Ошибка парсинга data.json: " + std::string(e.what())});
    }
    file.close();

    try {
        if (j.contains("accounts") && j["accounts"].is_array()) {
            for (auto& acc : j["accounts"]) {
                unsigned id  = acc.value("id", 0u);
                std::string name = acc.value("name", "");
                double balance = acc.value("balance", 0.0);
                std::string type = acc.value("type", "cash_account");
                Currency currency = StringToCurrency(acc.value("currency", "byn"));
                if (type == "cash_account") {
                    accounts.push_back(std::make_unique<CashAccount>(id, name, balance, currency));
                }
                else if (type == "bank_account") {
                    std::string last_four_digits = acc.value("last_four_digits", "");
                    accounts.push_back(std::make_unique<BankAccount>(id, name, balance, currency, last_four_digits));
                }
                else if (type == "savings_account") {
                    double goal_amount = acc.value("goal_amount", 0.0);
                    std::chrono::year_month_day deadline = ymd_from_string(acc.value("deadline", "2030-01-01"));
                    accounts.push_back(std::make_unique<SavingsAccount>(id, name, balance, currency, goal_amount, deadline));
                }
            }
        }
        if (j.contains("transactions") && j["transactions"].is_array()) {
            for (auto& t: j["transactions"]) {
                unsigned id = t.value("id", 0u);
                double amount = t.value("amount", 0.0);
                std::chrono::year_month_day date = ymd_from_string(t.value("date", "2026-01-01"));
                std::string type = t.value("type", "expense");
                unsigned category_id = t.value("category_id", 0u);
                unsigned account_id = t.value("account_id", 0u);
                if (type == "income") {
                    transactions.push_back(std::make_unique<Income>(id, amount, date, category_id, account_id ));
                }
                else if (type == "expense") {
                    transactions.push_back(std::make_unique<Expense>(id, amount, date, category_id, account_id ));
                }
                else if (type == "regular_expense") {
                    std::chrono::year_month_day next_date = ymd_from_string(t.value("next_date", "2024-01-01"));
                    Period period = String_to_period(t.value("period", "monthly"));
                    transactions.push_back(std::make_unique<RegularExpense>(id, amount, date, category_id, account_id, next_date, period));
                }
                else if (type == "transfer") {
                    unsigned destination_id = t.value("destination_id", 0u);
                    transactions.push_back(std::make_unique<Transfer>(id, amount, date, account_id, destination_id));
                }
            }
        }
        if (j.contains("categories") && j["categories"].is_array()) {
            for (auto& c : j["categories"]) {
                unsigned id = c.value("id", 0u);
                TransactionType type = t_type_from_string(c.value("transaction_type", "expense"));
                std::string name = c.value("name", "");
                std::string color = c.value("color", "#00ff66");
                categories.emplace_back(id, type, name, color);
            }
        }
        if (j.contains("budgets") && j["budgets"].is_array()) {
            for (auto& b: j["budgets"]) {
                unsigned category_id = b.value("category_id", 0u);
                double limit = b.value("limit", 0.0);
                double current_amount = b.value("current_amount", 0.0);
                for (Category& c : categories) {
                    if (c.id == category_id) {
                        budgets.emplace_back(c, limit, current_amount);
                        break;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        return std::unexpected(AppError{ErrorCode::ParseError, "Ошибка обработки данных JSON: " + std::string(e.what())});
    }
    return {};
}   
