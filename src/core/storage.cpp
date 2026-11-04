#include "storage.h"


//запись счетов
void Storage::save_accounts(json& j, const std::vector<std::unique_ptr<Account>> &accounts) {
    //создание json-массива
    json accs = json::array();
    //цикл по массиву счетов
    for (const auto& acc : accounts) {
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


//запись транзакций
void Storage::save_transactions(json& j, const std::vector<std::unique_ptr<Transaction>> &transactions) {
    //создание json-массива
    json trans = json::array();
    //цикл по транзакциям
    for (const auto& tran : transactions) {
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
            t["next_date"] =ymd_to_string(reg_exp->get_next_date());
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


//запись категорий
void Storage::save_categories(json& j, const std::vector<Category> &categories) {
    //создание json-массива
    json cats = json::array();
    //цикл по категориям
    for (Category cat : categories) {
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


//запись лимитов
void Storage::save_budgets(json& j, const std::vector<Budget> &budgets) {
    // создание json-массива
    json buds = json::array();
    //цикл по лимитам
    for (Budget bud : budgets) {
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

//подключение windows api при сборке под windows
#ifdef _WIN32
#include <windows.h>
#endif

std::expected<void, AppError> Storage::save(const std::vector<std::unique_ptr<Account>>& accounts,
                   const std::vector<std::unique_ptr<Transaction>>& transactions,
                   const std::vector<Category>& categories,
                   const std::vector<Budget>& budgets) {
    //создание json-объекта
    json j;
    //запись счетов, транзакций, категорий и лимитов
    save_accounts(j, accounts);
    save_transactions(j, transactions);
    save_categories(j, categories);
    save_budgets(j, budgets);

    //имя временного файла
    std::string temp_name = filename + ".tmp";
    //создание и открытие временного файла
    auto res = open_out_file(temp_name);
    if (!res) {
        //возврат ошибки
       return std::unexpected(res.error());
    }
    std::ofstream outfile = std::move(res.value());

    //запись информации в файл с сбросом буфера и закрытием
    outfile << j.dump(4);
    outfile.flush();
    outfile.close();

    //замена временного файла на целевой файл
    //вариант для windows
#ifdef _WIN32
    //преобразование путей в широкий формат для winapi
    std::wstring wtemp(temp_name.begin(), temp_name.end());
    std::wstring wfinal(filename.begin(), filename.end());
    //атомарная замена файла и принудительный сброс на диск
    if (!MoveFileExW(wtemp.c_str(), wfinal.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        //запасной вариант замены при ошибке winapi
        std::error_code ec;
        if (std::filesystem::exists(filename)) {
            //удаление старого файла
            std::filesystem::remove(filename, ec);
        }
        //переименование временного файла
        std::filesystem::rename(temp_name, filename, ec);
        if (ec) {
            return std::unexpected(AppError{ErrorCode::IoError, "Не удалось заменить файл базы данных: " + ec.message()});
        }
    }
    //вариант для остальных операционных систем (linux, macos)
#else
    std::error_code ec;
    //переименование временного файла
    std::filesystem::rename(temp_name, filename, ec);
    if (ec) {
        return std::unexpected(AppError{ErrorCode::IoError, "Не удалось заменить файл базы данных: " + ec.message()});
    }
#endif
    return {};
}



//загрузка информации из файла
//в случае ошибки возвращает std::unexpected<AppError>
std::expected<void, AppError> Storage::load(std::vector<std::unique_ptr<Account>>& accounts,
                   std::vector<std::unique_ptr<Transaction>>& transactions,
                   std::vector<Category>& categories,
                   std::vector<Budget>& budgets) {

    // Восстановление, если произошла ошибка сохранения файла и файл не существует
    if (!std::filesystem::exists(filename)) {
        //проверка на существование временного файла
        std::string temp_name = filename + ".tmp";
        if (std::filesystem::exists(temp_name)) {
            //переименование файла
            std::error_code ec;
            std::filesystem::rename(temp_name, filename, ec);
        }
        //если файла не существует
        if (!std::filesystem::exists(filename)) {
            return {};
        }
    }

    //создание и открытие временного файла
    auto res = open_in_file(filename);
    if (!res) {
        //возврат ошибки
        return std::unexpected(res.error());
    }
    std::ifstream file = std::move(res.value());

    //создание json-объекта для чтения
    json j;
    //попытка парса файла с отлавливанием исключений
    try {
        j = json::parse(file);
    } catch (const json::parse_error& e) {
        file.close();
        return std::unexpected(AppError{ErrorCode::ParseError, "Ошибка чтения data.json: неверный JSON синтаксис (" + std::string(e.what()) + ")"});
    } catch (const std::exception& e) {
        file.close();
        return std::unexpected(AppError{ErrorCode::ParseError, "Ошибка парсинга data.json: " + std::string(e.what())});
    }
    //закрытие файла
    file.close();

    //чтение json-объекта
    try {
        //проверка на существование поля счетов
        if (j.contains("accounts") && j["accounts"].is_array()) {
            //цикл по счетам
            for (auto& acc : j["accounts"]) {
                //заполнение базовых полей счета
                unsigned id  = acc.value("id", 0u);
                std::string name = acc.value("name", "");
                double balance = acc.value("balance", 0.0);
                std::string type = acc.value("type", "cash_account");
                Currency currency = StringToCurrency(acc.value("currency", "byn"));
                //проверка на тип счета, заполнение уникальных полей и добавление в массив нового элемента
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
        //если есть поле транзакций
        if (j.contains("transactions") && j["transactions"].is_array()) {
            //цикл по транзакциям
            for (auto& t: j["transactions"]) {
                //заполнение базовых полей транзакций
                unsigned id = t.value("id", 0u);
                double amount = t.value("amount", 0.0);
                std::chrono::year_month_day date = ymd_from_string(t.value("date", "2026-01-01"));
                std::string type = t.value("type", "expense");
                unsigned category_id = t.value("category_id", 0u);
                unsigned account_id = t.value("account_id", 0u);
                //проверка на тип транзакции, заполнение уникальных полей и добавление элемента в массив
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
        //если есть поле категорий
        if (j.contains("categories") && j["categories"].is_array()) {
            //цикл по категориям
            for (auto& c : j["categories"]) {
                //считывание информации о полях
                unsigned id = c.value("id", 0u);
                TransactionType type = t_type_from_string(c.value("transaction_type", "expense"));
                std::string name = c.value("name", "");
                std::string color = c.value("color", "#00ff66");
                //создание и добавление элемента в массив
                categories.emplace_back(id, type, name, color);
            }
        }
        //если есть поле лимитов
        if (j.contains("budgets") && j["budgets"].is_array()) {
            //цикл по лимитам
            for (auto& b: j["budgets"]) {
                //заполнение полей лимита
                unsigned category_id = b.value("category_id", 0u);
                double limit = b.value("limit", 0.0);
                double current_amount = b.value("current_amount", 0.0);
                //поиск категории по айди
                for (Category& c : categories) {
                    if (c.id == category_id) {
                        //создание и добавление лимита в массив
                        budgets.emplace_back(c, limit, current_amount);
                        break;
                    }
                }
            }
        }
        //возврат ошибки в случае исключения
    } catch (const std::exception& e) {
        return std::unexpected(AppError{ErrorCode::ParseError, "Ошибка обработки данных JSON: " + std::string(e.what())});
    }
    return {};
}
