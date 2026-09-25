#include "api_handler.h"


void ApiHandler::register_all() {
    //получение
    w.bind("getAccounts", [this](std::string seq) {
        return handle_get_accounts(seq);
    });
    w.bind("getTransactions", [this](std::string seq) {
        return handle_get_transactions(seq);
    });
    w.bind("getCategories", [this](std::string seq) {
        return handle_get_categories(seq);
    });
    w.bind("getBudgets", [this](std::string seq) {
        return handle_get_budgets(seq);
    });
    w.bind("getTransactionsByAccount", [this](std::string seq) {
        return handle_get_transactions_by_account(seq);
    });
    w.bind("getMonthlyDashboard", [this](std::string seq) {
        return handle_get_monthly_dashboard(seq);
    });
    w.bind("getCategoryReport", [this](std::string seq) {
        return handle_get_category_report(seq);
    });
    w.bind("getYearlyReport", [this](std::string seq) {
        return handle_get_yearly_report(seq);
    });
    //добавление
    w.bind("addAccount", [this](std::string seq) {
        return handle_add_account(seq);
    });
    w.bind("addTransaction", [this](std::string seq) {
        return handle_add_transaction(seq);
    });
    w.bind("addCategory", [this](std::string seq) {
        return handle_add_category(seq);
    });
    w.bind("addBudget", [this](std::string seq) {
        return handle_add_budget(seq);
    });
    //удаление
    w.bind("removeAccount", [this](std::string seq) {
        return handle_remove_account(seq);
    });
    w.bind("removeTransaction", [this](std::string seq) {
        return handle_remove_transaction(seq);
    });
    w.bind("removeCategory", [this](std::string seq) {
        return handle_remove_category(seq);
    });
    w.bind("removeBudget", [this](std::string seq) {
        return handle_remove_budget(seq);
    });
    //редактирование
    w.bind("editAccount", [this](std::string seq) {
        return handle_edit_account(seq);
    });
    w.bind("editBudget", [this](std::string seq) {
        return handle_edit_budget(seq);
    });
}


std::string ApiHandler::handle_get_accounts(std::string seq) {
    try {
        json response = json::array();
        for (auto* acc : manager.get_accounts()) { //[cite: 8]
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
                a["monthly_required"] = static_cast<int>(sav_acc->monthly_required()) +1;
                a["deadline"] = ymd_to_string(sav_acc->get_deadline());
            }

            response.push_back(a);
        }

        return response.dump();
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_get_transactions(std::string seq) {
    try {
        json response = json::array();
        for (auto * transaction : manager.get_transactions()) {
            json t;
            t["id"] = transaction->get_id();
            t["amount"] = transaction->get_amount();
            t["date"] = ymd_to_string(transaction->get_date());
            t["type"] = transaction->get_type();
            t["category_id"] = transaction->get_category_id();;
            t["account_id"] = transaction->get_account_id();
            if (auto* reg_exp = dynamic_cast<RegularExpense*>(transaction)) {
                t["next_date"] =ymd_to_string(reg_exp->get_next_date());
                t["period"] = Period_to_string(reg_exp->get_period());
            }
            else if (auto* transfer = dynamic_cast<Transfer*>(transaction)) {
                t["destination_id"] = transfer->get_destination_id();
            }
            response.push_back(t);
        }
        return  response.dump();
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_get_categories(std::string seq) {
    try {
        json response = json::array();
        for (auto&  category : manager.get_categories()) {
            json c;
            c["id"] = category.id;
            c["name"] = category.name;
            c["type"] = t_type_to_string(category.type);
            c["color"] = category.color;
            response.push_back(c);
        }
        return response.dump();
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_get_budgets(std::string seq) {
    try {
        json response = json::array();
        for (const auto& budget : manager.get_budgets()) { //[cite: 8]
            json b;
            b["category_id"] = budget.get_category().id; //[cite: 4]
            b["name"] = budget.get_category().name; //[cite: 4]
            b["color"] = budget.get_category().color; //[cite: 4]
            b["limit"] = budget.get_limit(); //[cite: 4]
            b["current_amount"] = budget.get_current_amount(); //[cite: 4]
            b["status"] = status_to_string(budget.get_status()); //[cite: 4]
            response.push_back(b);
        }
        return response.dump();
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_get_transactions_by_account(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data[0];
        json response = json::array();
        for (auto * transaction : manager.get_transactions()) {
            if (transaction->get_account_id() == id) {
                json t;
                t["id"] = transaction->get_id();
                t["amount"] = transaction->get_amount();
                t["date"] = ymd_to_string(transaction->get_date());
                t["type"] = transaction->get_type();
                t["category_id"] = transaction->get_category_id();;
                t["account_id"] = transaction->get_account_id();
                if (auto* reg_exp = dynamic_cast<RegularExpense*>(transaction)) {
                    t["next_date"] =ymd_to_string(reg_exp->get_next_date());
                    t["period"] = Period_to_string(reg_exp->get_period());
                }
                else if (auto* transfer = dynamic_cast<Transfer*>(transaction)) {
                    t["destination_id"] = transfer->get_destination_id();
                }
                response.push_back(t);
            }
        }
        return  response.dump();
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_get_monthly_dashboard(std::string seq) {
    try {
        auto now = std::chrono::system_clock::now();
        std::chrono::year_month_day today{std::chrono::floor<std::chrono::days>(now)};

        std::chrono::year_month_day first_day{today.year(), today.month(), std::chrono::day{1}};

        json report = manager.get_monthly_report(first_day, today);
        return report.dump();
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_get_category_report(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        auto from = ymd_from_string(data[0]);
        auto to = ymd_from_string(data[1]);
        json report = manager.get_category_report(from, to);
        return report.dump();
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_get_yearly_report(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        int year = data[0];
        json report = manager.get_yearly_report(year);
        return report.dump();
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_add_account(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned max_id = 0;
        for (const auto* a : manager.get_accounts()) {
            if (a->get_id() > max_id) max_id = a->get_id();
        }
        unsigned id = max_id + 1;
        std::string name = data["name"];
        double balance = data["balance"];
        Currency currency = StringToCurrency(data["currency"]);
        std::string type = data["type"];

        if (type == "cash_account") {
            manager.add_account(new CashAccount(id, name, balance, currency));
        } else if (type == "bank_account") {
            std::string digits = data["last_four_digits"];
            manager.add_account(new BankAccount(id, name, balance, currency, digits));
        } else if (type == "savings_account") {
            double goal = data["goal_amount"];
            std::chrono::year_month_day deadline = ymd_from_string(data["deadline"]);
            manager.add_account(new SavingsAccount(id, name, balance, currency, goal, deadline));
        }

        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_add_transaction(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned max_id = 0;
        for (const auto* t : manager.get_transactions()) {
            if (t->get_id() > max_id) max_id = t->get_id();
        }
        unsigned id = max_id + 1;
        double amount = data["amount"];
        auto date = ymd_from_string(data["date"]);
        std::string type = data["type"];
        unsigned account_id = data["account_id"];
        unsigned category_id = 0;

        if (type == "expense" || type == "regular_expense" || type == "transfer") {
            Account* acc = manager.get_account_by_id(account_id);
            if (acc->get_balance() < amount) {
                return "{\"error\":\"Недостаточно средств на счете!\"}";
            }
        }
        if (type != "transfer") category_id = data["category_id"];
        if ( type == "income") {
            manager.add_transaction(new Income(id, amount, date, category_id, account_id));
        }
        else if (type == "expense") {
            manager.add_transaction(new Expense (id, amount, date,  category_id, account_id) );
        }
        else if (type == "transfer") {
            unsigned destination_id = data["destination_id"];
            manager.add_transaction(new Transfer(id, amount, date, account_id, destination_id));
        }
        else if (type == "regular_expense") {
            Period p = String_to_period(data["period"]);
            manager.add_transaction(new RegularExpense(id, amount, date, category_id, account_id, date, p));
        }
        return "{\"status\":\"success\"}";
    }
    catch(const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_add_category(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];

        unsigned max_id = 0;
        for (const auto& c : manager.get_categories()) {
            if (c.id > max_id) max_id = c.id;
        }
        unsigned id = max_id + 1;
        std::string name = data["name"];
        std::string type_str = data["type"];
        std::string color = data["color"];

        TransactionType type = t_type_from_string(type_str);
        Category new_cat{id, type, name, color};

        manager.add_category(new_cat);

        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_add_budget(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];

        unsigned cat_id = data["category_id"];
        double limit = data["limit"];

        Category cat = manager.get_category_by_id(cat_id); //[cite: 7, 8]
        manager.add_budget(Budget(cat, limit, 0.0)); //[cite: 4, 7, 8]

        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_remove_account(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data[0];
        manager.remove_account(id);
        return "{\"status\":\"success\"}";
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_remove_transaction(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data[0];
        manager.remove_transaction(id);
        return "{\"status\":\"success\"}";
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_remove_category(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data[0];
        manager.remove_category(id);
        return "{\"status\":\"success\"}";
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_remove_budget(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data[0];
        manager.remove_budget(id);
        return "{\"status\":\"success\"}";
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_edit_account(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        Account* edit_acc = manager.get_account_by_id(data["id"]);
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
        manager.save_data();
        return "{\"status\":\"success\"}";
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_edit_budget(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        manager.edit_budget(data["category_id"], data["limit"]);
        return "{\"status\":\"success\"}";
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}
