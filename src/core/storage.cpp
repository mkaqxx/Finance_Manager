#include "storage.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

//преобразование utf-8 строки в широкую строку для поддержки кириллицы в windows
static std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
    if (len <= 0) return std::wstring(str.begin(), str.end());
    std::wstring w(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), w.data(), len);
    return w;
}
#endif

//запись счетов в json
void Storage::save_accounts(json& j, const std::vector<std::unique_ptr<Account>> &accounts) {
    //создание json-массива
    json accs = json::array();
    //цикл по массиву счетов
    for (const auto& acc : accounts) {
        if (!acc) continue;
        //заполнение одного элемента json-массива
        json a;
        a["id"] = acc->get_id();
        a["name"] = acc->get_name();
        a["balance"] = acc->get_balance();
        a["type"] = acc->get_type();
        a["currency"] = CurrencyToString(acc->get_currency());
        //проверки на тип счета и заполнение уникальных полей
        if (auto* bank_acc = dynamic_cast<BankAccount*>(acc.get())) {
            a["last_four_digits"] = bank_acc->get_last_four_digits();
        }
        else if (auto* save_acc = dynamic_cast<SavingsAccount*>(acc.get())) {
            a["goal_amount"] = save_acc->get_goal_amount();
            a["deadline"] = ymd_to_string(save_acc->get_deadline());
        }
        //добавление в массив
        accs.push_back(a);
    }
    //запись информации в общий json-объект
    j["accounts"] = accs;
}

//запись транзакций в json
void Storage::save_transactions(json& j, const std::vector<std::unique_ptr<Transaction>> &transactions) {
    //создание json-массива
    json trans = json::array();
    //цикл по транзакциям
    for (const auto& tran : transactions) {
        if (!tran) continue;
        //заполнение информации об одной транзакции
        json t;
        t["id"] = tran->get_id();
        t["amount"] = tran->get_amount();
        t["date"] = ymd_to_string(tran->get_date());
        t["type"] = tran->get_type();
        t["category_id"] = tran->get_category_id();
        t["account_id"] = tran->get_account_id();
        //проверка типа транзакции и заполнение уникальных полей
        if (auto* reg_exp = dynamic_cast<RegularExpense*>(tran.get())) {
            t["next_date"] = ymd_to_string(reg_exp->get_next_date());
            t["period"] = Period_to_string(reg_exp->get_period());
        }
        else if (auto* transfer = dynamic_cast<Transfer*>(tran.get())) {
            t["destination_id"] = transfer->get_destination_id();
        }
        //добавление элемента в массив
        trans.push_back(t);
    }
    //запись массива в поле транзакций
    j["transactions"] = trans;
}

//запись категорий в json
void Storage::save_categories(json& j, const std::vector<Category> &categories) {
    //создание json-массива
    json cats = json::array();
    //цикл по категориям
    for (const Category& cat : categories) {
        //заполнение одной категории
        json c;
        c["id"] = cat.id;
        c["transaction_type"] = t_type_to_string(cat.type);
        c["name"] = cat.name;
        c["color"] = cat.color;
        //добавление категории в массив
        cats.push_back(c);
    }
    //запись массива в поле категорий
    j["categories"] = cats;
}

//запись лимитов бюджетов в json
void Storage::save_budgets(json& j, const std::vector<Budget> &budgets) {
    //создание json-массива
    json buds = json::array();
    //цикл по лимитам
    for (const Budget& bud : budgets) {
        //заполнение одного лимита
        json b;
        b["category_id"] = bud.get_category().id;
        b["limit"] = bud.get_limit();
        b["current_amount"] = bud.get_current_amount();
        //добавление элемента в массив
        buds.push_back(b);
    }
    //запись массива в поле лимитов
    j["budgets"] = buds;
}

//синхронизация данных с indexeddb браузера в среде webassembly
void Storage::sync_to_database() noexcept {
#ifdef __EMSCRIPTEN__
    //вызов emscripten api fs.syncfs(false, callback) для записи memfs в indexeddb
    EM_ASM({
        if (typeof FS !== 'undefined' && FS.syncfs) {
            FS.syncfs(false, function (err) {
                if (err) {
                    console.error('[IDBFS] syncfs error:', err);
                } else {
                    console.log('[IDBFS] Data persisted to IndexedDB.');
                }
            });
        }
    });
#endif
}

//сохранение данных приложения в файл
std::expected<void, AppError> Storage::save(const std::vector<std::unique_ptr<Account>>& accounts,
                                           const std::vector<std::unique_ptr<Transaction>>& transactions,
                                           const std::vector<Category>& categories,
                                           const std::vector<Budget>& budgets) {
    //создание корневого json-объекта
    json j;
    //запись счетов, транзакций, категорий и лимитов
    save_accounts(j, accounts);
    save_transactions(j, transactions);
    save_categories(j, categories);
    save_budgets(j, budgets);

    //создание родительской папки (например, /data в виртуальной файловой системе memfs)
    std::filesystem::path dir = std::filesystem::path(filename).parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::error_code dir_ec;
        std::filesystem::create_directories(dir, dir_ec);
    }

    //имя временного файла
    std::string temp_name = filename + ".tmp";
    //создание и открытие временного файла
    auto res = open_out_file(temp_name);
    if (!res) {
        return std::unexpected(res.error());
    }

    //запись json с отступами и сброс буфера на диск
    std::ofstream outfile = std::move(res.value());
    outfile << j.dump(4);
    outfile.flush();
    if (outfile.fail()) {
        outfile.close();
        std::error_code ec;
        std::filesystem::remove(temp_name, ec);
        return std::unexpected(AppError{ErrorCode::IoError, "Ошибка записи данных во временный файл"});
    }
    outfile.close();

    //атомарная замена временного файла на основной
#ifdef _WIN32
    //преобразование путей в широкий формат для winapi с поддержкой utf-8
    std::wstring w_tmp = utf8_to_wstring(temp_name);
    std::wstring w_dest = utf8_to_wstring(filename);
    if (!MoveFileExW(w_tmp.c_str(), w_dest.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        return std::unexpected(AppError{ErrorCode::IoError, "Ошибка перемещения временного файла данных (код Windows: " + std::to_string(GetLastError()) + ")"});
    }
#else
    std::error_code ec;
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename, ec);
    }
    std::filesystem::rename(temp_name, filename, ec);
    if (ec) {
        return std::unexpected(AppError{ErrorCode::IoError, "Ошибка перемещения временного файла данных: " + ec.message()});
    }
#endif

    //синхронизация виртуальной файловой системы emscripten с indexeddb браузера
    sync_to_database();

    return {};
}

//загрузка данных из файла
std::expected<void, AppError> Storage::load(std::vector<std::unique_ptr<Account>>& accounts,
                                           std::vector<std::unique_ptr<Transaction>>& transactions,
                                           std::vector<Category>& categories,
                                           std::vector<Budget>& budgets) {
    std::string temp_name = filename + ".tmp";

    //восстановление из .tmp при сбое предыдущего сохранения
    if (!std::filesystem::exists(filename) && std::filesystem::exists(temp_name)) {
#ifdef _WIN32
        //восстановление файла с поддержкой путей utf-8
        std::wstring w_tmp = utf8_to_wstring(temp_name);
        std::wstring w_dest = utf8_to_wstring(filename);
        MoveFileExW(w_tmp.c_str(), w_dest.c_str(), MOVEFILE_REPLACE_EXISTING);
#else
        std::error_code ec;
        std::filesystem::rename(temp_name, filename, ec);
#endif
    }

    //если файла базы еще нет (первый запуск)
    if (!std::filesystem::exists(filename)) {
        return {};
    }

    //открытие файла для чтения
    auto res = open_in_file(filename);
    if (!res) {
        return std::unexpected(res.error());
    }

    //чтение и парсинг json
    std::ifstream file = std::move(res.value());
    json j;
    try {
        j = json::parse(file);
    } catch (const json::parse_error& e) {
        file.close();
        return std::unexpected(AppError{ErrorCode::ParseError, "Ошибка синтаксиса JSON в файле хранилища: " + std::string(e.what())});
    }
    file.close();

    //извлечение данных из json-объекта
    try {
        //чтение счетов
        if (j.contains("accounts") && j["accounts"].is_array()) {
            for (auto& acc : j["accounts"]) {
                unsigned id = acc.value("id", 0u);
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
                    std::chrono::year_month_day deadline = ymd_from_string(acc.value("deadline", "2099-01-01"));
                    accounts.push_back(std::make_unique<SavingsAccount>(id, name, balance, currency, goal_amount, deadline));
                }
            }
        }

        //чтение транзакций
        if (j.contains("transactions") && j["transactions"].is_array()) {
            for (auto& t: j["transactions"]) {
                unsigned id = t.value("id", 0u);
                double amount = t.value("amount", 0.0);
                std::chrono::year_month_day date = ymd_from_string(t.value("date", "2000-01-01"));
                std::string type = t.value("type", "expense");
                unsigned category_id = t.value("category_id", 0u);
                unsigned account_id = t.value("account_id", 0u);
                if (type == "income") {
                    transactions.push_back(std::make_unique<Income>(id, amount, date, category_id, account_id));
                }
                else if (type == "expense") {
                    transactions.push_back(std::make_unique<Expense>(id, amount, date, category_id, account_id));
                }
                else if (type == "regular_expense") {
                    std::chrono::year_month_day next_date = ymd_from_string(t.value("next_date", "2000-01-01"));
                    Period period = String_to_period(t.value("period", "monthly"));
                    transactions.push_back(std::make_unique<RegularExpense>(id, amount, date, category_id, account_id, next_date, period));
                }
                else if (type == "transfer") {
                    unsigned destination_id = t.value("destination_id", 0u);
                    transactions.push_back(std::make_unique<Transfer>(id, amount, date, account_id, destination_id));
                }
            }
        }

        //чтение категорий
        if (j.contains("categories") && j["categories"].is_array()) {
            for (auto& c : j["categories"]) {
                unsigned id = c.value("id", 0u);
                TransactionType type = t_type_from_string(c.value("transaction_type", "expense"));
                std::string name = c.value("name", "");
                std::string color = c.value("color", "#00ff66");
                categories.emplace_back(id, type, name, color);
            }
        }

        //чтение лимитов бюджетов
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
