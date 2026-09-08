#pragma once
#include <fstream>
#include <vector>
#include "../json.hpp"
using json = nlohmann::json;
#include "../models/account.h"
#include "../models/budget.h"
#include "../utils/chrono_to_string.h"



class Storage {
    public:
    void load(std::vector<Account*>& accounts,
               std::vector<Transaction*>& transactions,
               std::vector<Category>& categories,
               std::vector<Budget>& budgets);

    void save(const std::vector<Account*>& accounts,
              const std::vector<Transaction*>& transactions,
              const std::vector<Category>& categories,
              const std::vector<Budget>& budgets);
    private:
    void save_accounts(json& j, const std::vector<Account*>& accounts);
    void save_transactions(json& j, const std::vector<Transaction*>& transactions);
    void save_categories(json& j, const std::vector<Category>& categories);
    void save_budgets(json& j, const std::vector<Budget>& budgets);
    std::string filename = "data.json";
};

