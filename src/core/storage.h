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



class Storage {
    public:
     std::expected<void, int> load(std::vector<std::unique_ptr<Account>>& accounts,
               std::vector<std::unique_ptr<Transaction>>& transactions,
               std::vector<Category>& categories,
               std::vector<Budget>& budgets);

    std::expected<void, int> save(const std::vector<std::unique_ptr<Account>>& accounts,
              const std::vector<std::unique_ptr<Transaction>>& transactions,
              const std::vector<Category>& categories,
              const std::vector<Budget>& budgets);
    private:
    void save_accounts(json& j, const std::vector<std::unique_ptr<Account>>& accounts);
    void save_transactions(json& j, const std::vector<std::unique_ptr<Transaction>>& transactions);
    void save_categories(json& j, const std::vector<Category>& categories);
    void save_budgets(json& j, const std::vector<Budget>& budgets);
    std::string filename = "data.json";
};

