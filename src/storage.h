#pragma once
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;
#include "./models/account.h"
#include "./models/budget.h"


class Finance_manager;


class Storage {
    public:
    void load(Finance_manager &manager);
    void save(Finance_manager &manager);
    private:
    void save_accounts(json& j, const std::vector<Account*>& accounts);
    void save_transactions(json& j, const std::vector<Transaction*>& transactions);
    void save_categories(json& j, const std::vector<Category>& categories);
    void save_budgets(json& j, const std::vector<Budget>& budgets);
    std::string filename = "data.json";
};

