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


#include <cmath>

std::string ApiHandler::handle_get_accounts(std::string seq) {
    (void)seq;
    try {
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


std::string ApiHandler::handle_get_transactions(std::string seq) {
    (void)seq;
    try {
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
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_get_categories(std::string seq) {
    (void)seq;
    try {
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
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_get_budgets(std::string seq) {
    (void)seq;
    try {
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
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_get_transactions_by_account(std::string seq) {
    try {
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


std::string ApiHandler::handle_get_monthly_dashboard(std::string seq) {
    (void)seq;
    try {
        auto now = std::chrono::system_clock::now();
        std::chrono::year_month_day today{std::chrono::floor<std::chrono::days>(now)};
        std::chrono::year_month_day first_day{today.year(), today.month(), std::chrono::day{1}};

        json report = manager.get_monthly_report(first_day, today);
        return report.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
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
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_get_yearly_report(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        int year = data[0];
        json report = manager.get_yearly_report(year);
        return report.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
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
        if (name.empty()) {
            return json{{"error", "Название счёта не может быть пустым"}}.dump();
        }
        double balance = data["balance"];
        if (std::isnan(balance) || std::isinf(balance) || balance < 0.0) {
            return json{{"error", "Баланс счёта должен быть неотрицательным числом"}}.dump();
        }
        Currency currency = StringToCurrency(data["currency"]);
        std::string type = data["type"];

        std::expected<void, AppError> res;
        if (type == "cash_account") {
            res = manager.add_account(std::make_unique<CashAccount>(id, name, balance, currency));
        } else if (type == "bank_account") {
            std::string digits = data["last_four_digits"];
            res = manager.add_account(std::make_unique<BankAccount>(id, name, balance, currency, digits));
        } else if (type == "savings_account") {
            double goal = data["goal_amount"];
            if (std::isnan(goal) || std::isinf(goal) || goal <= 0.0) {
                return json{{"error", "Целевая сумма накопления должна быть больше нуля"}}.dump();
            }
            std::chrono::year_month_day deadline = ymd_from_string(data["deadline"]);
            res = manager.add_account(std::make_unique<SavingsAccount>(id, name, balance, currency, goal, deadline));
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
        if (std::isnan(amount) || std::isinf(amount) || amount <= 0.0) {
            return json{{"error", "Сумма операции должна быть больше нуля"}}.dump();
        }

        auto date = ymd_from_string(data["date"]);
        std::string type = data["type"];
        unsigned account_id = data["account_id"];
        unsigned category_id = 0;

        if (type == "expense" || type == "regular_expense" || type == "transfer") {
            Account* acc = manager.get_account_by_id(account_id);
            if (!acc) {
                return json{{"error", "Счёт списания не найден"}}.dump();
            }
            if (acc->get_balance() < amount) {
                return json{{"error", "Недостаточно средств на счёте"}}.dump();
            }
        }
        if (type != "transfer") category_id = data["category_id"];
        std::expected<void, AppError> res;
        if (type == "income") {
            res = manager.add_transaction(std::make_unique<Income>(id, amount, date, category_id, account_id));
        }
        else if (type == "expense") {
            res = manager.add_transaction(std::make_unique<Expense>(id, amount, date, category_id, account_id));
        }
        else if (type == "transfer") {
            unsigned destination_id = data["destination_id"];
            if (account_id == destination_id) {
                return json{{"error", "Счёт списания и счёт зачисления не могут совпадать"}}.dump();
            }
            Account* src = manager.get_account_by_id(account_id);
            Account* dest = manager.get_account_by_id(destination_id);
            if (src && dest) {
                if (src->get_currency() != dest->get_currency()) {
                    return json{{"error", "Нельзя переводить между счетами с разными валютами"}}.dump();
                }
                res = manager.add_transaction(std::make_unique<Transfer>(id, amount, date, account_id, destination_id));
            }
            else {
                return json{{"error", "Один из счетов для перевода не найден"}}.dump();
            }
        }
        else if (type == "regular_expense") {
            Period p = String_to_period(data["period"]);
            auto reg = std::make_unique<RegularExpense>(id, amount, date, category_id, account_id, date, p);
            reg->update_next_date(); // Первое списание сейчас, следующее - через период
            res = manager.add_transaction(std::move(reg));
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
        if (name.empty()) {
            return json{{"error", "Название категории не может быть пустым"}}.dump();
        }
        std::string type_str = data["type"];
        std::string color = data["color"];

        TransactionType type = t_type_from_string(type_str);
        Category new_cat{id, type, name, color};

        auto res = manager.add_category(new_cat);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }

        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_add_budget(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];

        unsigned cat_id = data["category_id"];
        double limit = data["limit"];
        if (std::isnan(limit) || std::isinf(limit) || limit <= 0.0) {
            return json{{"error", "Лимит бюджета должен быть положительным числом"}}.dump();
        }
        double current_amount = 0.0;
        for (const auto* t: manager.get_transactions_raw()) {
            if (t->get_category_id() == cat_id && (t->get_type()=="expense" || t->get_type()=="regular_expense")) {
                Account* acc = manager.get_account_by_id(t->get_account_id());
                if (acc && acc->get_currency() == Currency::BYN) {
                    current_amount += t->get_amount();
                }
            }
        }
        Category cat = manager.get_category_by_id(cat_id);
        auto res = manager.add_budget(Budget(cat, limit, current_amount));
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }

        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_remove_account(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data[0];
        auto res = manager.remove_account(id);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_remove_transaction(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data[0];
        auto res = manager.remove_transaction(id);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_remove_category(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data[0];
        auto res = manager.remove_category(id);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_remove_budget(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data[0];
        auto res = manager.remove_budget(id);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_edit_account(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned id = data["id"];
        std::string name = data["name"];
        if (name.empty()) {
            return json{{"error", "Название счёта не может быть пустым"}}.dump();
        }
        double balance = data["balance"];
        if (std::isnan(balance) || std::isinf(balance) || balance < 0.0) {
            return json{{"error", "Баланс счёта должен быть неотрицательным числом"}}.dump();
        }
        Currency currency = StringToCurrency(data["currency"]);
        std::string type = data.value("type", "cash_account");
        std::string last_four_digits = "";
        double goal_amount = 0.0;
        std::chrono::year_month_day deadline{std::chrono::year{2030}, std::chrono::month{1}, std::chrono::day{1}};
        if (type == "bank_account") {
            last_four_digits = data.value("last_four_digits", "");
        } else if (type == "savings_account") {
            goal_amount = data.value("goal_amount", 0.0);
            if (std::isnan(goal_amount) || std::isinf(goal_amount) || goal_amount <= 0.0) {
                return json{{"error", "Целевая сумма накопления должна быть больше нуля"}}.dump();
            }
            if (data.contains("deadline")) {
                deadline = ymd_from_string(data["deadline"]);
            }
        }

        auto res = manager.edit_account(id, name, balance, currency, last_four_digits, goal_amount, deadline);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    }
    catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}


std::string ApiHandler::handle_edit_budget(std::string seq) {
    try {
        json args = json::parse(seq);
        json data = args[0];
        unsigned cat_id = data["category_id"];
        double limit = data["limit"];
        if (std::isnan(limit) || std::isinf(limit) || limit <= 0.0) {
            return json{{"error", "Лимит бюджета должен быть положительным числом"}}.dump();
        }
        auto res = manager.edit_budget(cat_id, limit);
        if (!res) {
            return json{{"error", res.error().message}}.dump();
        }
        return json{{"status", "success"}}.dump();
    } catch (const std::exception& e) {
        return json{{"error", e.what()}}.dump();
    }
}
