#pragma once

#include <vector>
#include <memory>
#include <expected>
#include <filesystem>
#include "../json.hpp"
#include "../models/account.h"
#include "../models/budget.h"
#include "../utils/chrono_to_string.h"
#include "../utils/safe_open_file.h"

using json = nlohmann::json;


//класс для управления хранилищем
class Storage {
    public:
    // загрузка из файла. при ошибке загрузки вместо выброса
    // исключения возвращает std::unexpected – код ошибки и информацию о ней
    std::expected<void, AppError> load(std::vector<std::unique_ptr<Account>>& accounts,
               std::vector<std::unique_ptr<Transaction>>& transactions,
               std::vector<Category>& categories,
               std::vector<Budget>& budgets);

    // сохранение в файл. при ошибке сохранения вместо выброса
    // исключения возвращает std::unexpected – код ошибки и информацию о ней
    std::expected<void, AppError> save(const std::vector<std::unique_ptr<Account>>& accounts,
              const std::vector<std::unique_ptr<Transaction>>& transactions,
              const std::vector<Category>& categories,
              const std::vector<Budget>& budgets);
    private:
    //вспомогательные методы для сохранения
    //каждый метод записывает только свою информацию
    void save_accounts(json& j, const std::vector<std::unique_ptr<Account>>& accounts);
    void save_transactions(json& j, const std::vector<std::unique_ptr<Transaction>>& transactions);
    void save_categories(json& j, const std::vector<Category>& categories);
    void save_budgets(json& j, const std::vector<Budget>& budgets);
    //имя файла
    std::string filename = "data.json";
};

