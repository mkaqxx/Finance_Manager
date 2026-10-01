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
    (void)seq;
    json response = json::array();
    for (auto* acc : manager.get_accounts_raw()) {
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


std::string ApiHandler::handle_get_transactions(std::string seq) {
    (void)seq;
    json response = json::array();
    for (auto* transaction : manager.get_transactions_raw()) {
        if (!transaction) continue;
        json t;
        t["id"] = transaction->get_id();
        t["amount"] = transaction->get_amount();
        t["date"] = ymd_to_string(transaction->get_date());
        t["type"] = transaction->get_type();
        t["category_id"] = transaction->get_category_id();
        t["account_id"] = transaction->get_account_id();
        Account* acc = manager.get_account_by_id(transaction->get_account_id());
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


std::string ApiHandler::handle_get_categories(std::string seq) {
    (void)seq;
    json response = json::array();
    for (auto& category : manager.get_categories()) {
        json c;
        c["id"] = category.id;
        c["name"] = category.name;
        c["type"] = t_type_to_string(category.type);
        c["color"] = category.color;
        response.push_back(c);
    }
    return response.dump();
}


std::string ApiHandler::handle_get_budgets(std::string seq) {
    (void)seq;
    json response = json::array();
    for (const auto& budget : manager.get_budgets()) {
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


std::string ApiHandler::handle_get_transactions_by_account(std::string seq) {
    json args = json::parse(seq);
    json data = args[0];
    unsigned id = data[0];
    json response = json::array();
    for (auto* transaction : manager.get_transactions_raw()) {
        if (!transaction) continue;
        if (transaction->get_account_id() == id) {
            json t;
            t["id"] = transaction->get_id();
            t["amount"] = transaction->get_amount();
            t["date"] = ymd_to_string(transaction->get_date());
            t["type"] = transaction->get_type();
            t["category_id"] = transaction->get_category_id();
            t["account_id"] = transaction->get_account_id();
            Account* acc = manager.get_account_by_id(transaction->get_account_id());
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


std::string ApiHandler::handle_get_monthly_dashboard(std::string seq) {
    (void)seq;
    auto now = std::chrono::system_clock::now();
    std::chrono::year_month_day today{std::chrono::floor<std::chrono::days>(now)};
    std::chrono::year_month_day first_day{today.year(), today.month(), std::chrono::day{1}};

    json report = manager.get_monthly_report(first_day, today);
    return report.dump();
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
    json args = json::parse(seq);
    json data = args[0];
    int year = data[0];
    json report = manager.get_yearly_report(year);
    return report.dump();
}


std::string ApiHandler::handle_add_account(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned max_id = 0;
        for (const auto* a : manager.get_accounts_raw()) {
            if (a->get_id() > max_id) max_id = a->get_id();
        }
        unsigned id = max_id + 1;
        std::string name = data["name"];
        double balance = data["balance"];
        Currency currency = StringToCurrency(data["currency"]);
        std::string type = data["type"];

        std::expected<void, int> res;
        if (type == "cash_account") {
            res = manager.add_account(std::make_unique<CashAccount>(id, name, balance, currency));
        } else if (type == "bank_account") {
            std::string digits = data["last_four_digits"];
            res = manager.add_account(std::make_unique<BankAccount>(id, name, balance, currency, digits));
        } else if (type == "savings_account") {
            double goal = data["goal_amount"];
            std::chrono::year_month_day deadline = ymd_from_string(data["deadline"]);
            res = manager.add_account(std::make_unique<SavingsAccount>(id, name, balance, currency, goal, deadline));
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


std::string ApiHandler::handle_add_transaction(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned max_id = 0;
        for (const auto* t : manager.get_transactions_raw()) {
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
            if (if acc && acc->get_balance() < amount) {
                return "{\"error\":\"Недостаточно средств на счете!\"}";
            }
        }
        if (type != "transfer") category_id = data["category_id"];
        std::expected<void, int> res;
        if (type == "income") {
            res = manager.add_transaction(std::make_unique<Income>(id, amount, date, category_id, account_id));
        }
        else if (type == "expense") {
            res = manager.add_transaction(std::make_unique<Expense>(id, amount, date, category_id, account_id));
        }
        else if (type == "transfer") {
            unsigned destination_id = data["destination_id"];
            Account* src = manager.get_account_by_id(account_id);
            Account* dest = manager.get_account_by_id(destination_id);
            if (src && dest) {
                if (src->get_currency() != dest->get_currency()) {
                    return "{\"error\":\"Нельзя переводить между счетами с разными валютами\"}";
                }
                res = manager.add_transaction(std::make_unique<Transfer>(id, amount, date, account_id, destination_id));
            }
            else return "{\"error:\" :\" аккаунт не найден или поврежден\"}";
        }
        else if (type == "regular_expense") {
            Period p = String_to_period(data["period"]);
            res = manager.add_transaction(std::make_unique<RegularExpense>(id, amount, date, category_id, account_id, date, p));
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

        auto res = manager.add_category(new_cat);
        if (!res) {
            return "{\"error\":\"Ошибка добавления категории (код: " + std::to_string(res.error()) + ")\"}";
        }

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
        double current_amount = 0.0;
        for (const auto* t: manager.get_transactions_raw()) {
            if (t->get_category_id() == cat_id && (t->get_type()=="expense" || t->get_type()=="regular_expense")) {
                Account* acc = manager.get_account_by_id(t->get_account_id());
                // Лимиты считаются только для базовой валюты (BYN)
                if (acc && acc->get_currency() == Currency::BYN) {
                    current_amount += t->get_amount();
                }
            }
        }
        Category cat = manager.get_category_by_id(cat_id);
        auto res = manager.add_budget(Budget(cat, limit, current_amount));
        if (!res) {
            return "{\"error\":\"Ошибка добавления бюджета (код: " + std::to_string(res.error()) + ")\"}";
        }

        return "{\"status\":\"success\"}";
    } catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_remove_account(std::string seq) {
    json args = json::parse(seq);
    json data = args[0];
    unsigned id = data[0];
    auto res = manager.remove_account(id);
    if (!res) {
        return "{\"error\":\"Ошибка удаления счёта (код: " + std::to_string(res.error()) + ")\"}";
    }
    return "{\"status\":\"success\"}";
}


std::string ApiHandler::handle_remove_transaction(std::string seq) {
    json args = json::parse(seq);
    json data = args[0];
    unsigned id = data[0];
    auto res = manager.remove_transaction(id);
    if (!res) {
        return "{\"error\":\"Ошибка удаления транзакции (код: " + std::to_string(res.error()) + ")\"}";
    }
    return "{\"status\":\"success\"}";
}


std::string ApiHandler::handle_remove_category(std::string seq) {
    json args = json::parse(seq);
    json data = args[0];
    unsigned id = data[0];
    auto res = manager.remove_category(id);
    if (!res) {
        return "{\"error\":\"Ошибка удаления категории (код: " + std::to_string(res.error()) + ")\"}";
    }
    return "{\"status\":\"success\"}";
}


std::string ApiHandler::handle_remove_budget(std::string seq) {
    json args = json::parse(seq);
    json data = args[0];
    unsigned id = data[0];
    auto res = manager.remove_budget(id);
    if (!res) {
        return "{\"error\":\"Ошибка удаления бюджета (код: " + std::to_string(res.error()) + ")\"}";
    }
    return "{\"status\":\"success\"}";
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
        auto res = manager.save_data();
        if (!res) {
            return "{\"error\":\"Ошибка сохранения счёта (код: " + std::to_string(res.error()) + ")\"}";
        }
        return "{\"status\":\"success\"}";
    }
    catch (const std::exception& e) {
        return "{\"error\":\"" + std::string(e.what()) + "\"}";
    }
}


std::string ApiHandler::handle_edit_budget(std::string seq) {
    json args = json::parse(seq);
    json data = args[0];
    auto res = manager.edit_budget(data["category_id"], data["limit"]);
    if (!res) {
        return "{\"error\":\"Ошибка обновления бюджета (код: " + std::to_string(res.error()) + ")\"}";
    }
    return "{\"status\":\"success\"}";
}
