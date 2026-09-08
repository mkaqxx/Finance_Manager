#include "storage.h"
#include "finance_manager.h"


void Storage::save_accounts(json& j, const std::vector<Account *> &accounts) {
    json accs = json::array();
    for (Account* acc : accounts) {
        json a;
        a["id"] = acc->get_id();
        a["name"] = acc->get_name();
        a["balance"] = acc->get_balance();
        a["type"] = acc->get_type();
        a["currency"] = CurrencyToString(acc->get_currency());
        if (auto* bank_acc = dynamic_cast<BankAccount*>(acc)) {
            a["last_four_digits"] = bank_acc->get_last_four_digits();
        }
        else if (auto* save_acc = dynamic_cast<SavingsAccount*>(acc)) {
            a["goal_amount"] = save_acc->get_goal_amount();
            a["deadline"] = ymd_to_string(save_acc->get_deadline());
        }

        accs.push_back(a);
    }
    j["accounts"] = accs;

}


void Storage::save_transactions(json& j, const std::vector<Transaction *> &transactions) {
    json trans = json::array();
    for (Transaction* tran : transactions) {
        json t;
        t["id"] = tran->get_id();
        t["amount"] = tran->get_amount();
        t["date"] = ymd_to_string(tran->get_date());
        t["type"] = tran->get_type();
        t["category_id"] = tran->get_category_id();
        t["account_id"] = tran->get_account_id();
        if (auto* reg_exp = dynamic_cast<RegularExpense*>(tran)) {
            t["next_date"] =ymd_to_string(reg_exp->get_next_date());
            t["period"] = Period_to_string(reg_exp->get_period());
        }
        else if (auto* transfer = dynamic_cast<Transfer*>(tran)) {
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

void Storage::save(Finance_manager &manager) {
    json j;
    std::ifstream infile(filename);
    if (infile.is_open()) {
        j = json::parse(infile);
        infile.close();
    }

    if (manager.accounts_changed) {
        save_accounts(j, manager.get_accounts());
        manager.accounts_changed = false;
    }
    if (manager.transactions_changed) {
        save_transactions(j, manager.get_transactions());
        manager.transactions_changed = false;
    }
    if (manager.categories_changed) {
        save_categories(j, manager.get_categories());
        manager.categories_changed = false;
    }
    if (manager.budgets_changed) {
        save_budgets(j, manager.get_budgets());
        manager.budgets_changed = false;
    }
    std::ofstream outfile (filename);
    if (!outfile.is_open()) {
        throw std::runtime_error("Could not open file");
    };
    outfile<<j.dump(4);
    outfile.flush();
    outfile.close();
}


void Storage::load(Finance_manager &manager) {
    std::ifstream file(filename);
    if (!file.is_open()) return;
    json j = json::parse(file);
    file.close();

    for (auto& acc : j["accounts"]) {
        unsigned id  = acc["id"];
        std::string name = acc["name"];
        double balance = acc["balance"];
        std::string type = acc["type"];
        Currency currency = StringToCurrency(acc["currency"]);
        if (type == "cash_account") {
            Account* new_acc = new CashAccount(id, name, balance, currency);
            manager.add_account_internal(new_acc);
        }
        else if (type == "bank_account") {
            std::string last_four_digits = acc["last_four_digits"];
            Account* new_acc = new BankAccount(id, name, balance, currency, last_four_digits);
            manager.add_account_internal(new_acc);
        }
        else if (type == "savings_account") {
            double goal_amount = acc["goal_amount"];
            std::chrono::year_month_day deadline = ymd_from_string(acc["deadline"]);
            Account* new_acc = new SavingsAccount(id, name, balance, currency, goal_amount, deadline);
            manager.add_account_internal(new_acc);
        }
    }
    for (auto& t: j["transactions"]) {
        unsigned id = t["id"];
        double amount = t["amount"];
        std::chrono::year_month_day date = ymd_from_string(t["date"]);
        std::string type = t["type"];
        unsigned category_id = t["category_id"];
        unsigned account_id = t["account_id"];
        if (type == "income") {
            Transaction* new_tran = new Income(id, amount, date, category_id, account_id );
            manager.add_transaction_internal(new_tran);
        }
        else if (type == "expense") {
            Transaction* new_tran = new Expense(id, amount, date, category_id, account_id );
            manager.add_transaction_internal(new_tran);
        }
        else if (type== "regular_expense") {
            std::chrono::year_month_day next_date = ymd_from_string(t["next_date"]);
            Period period = String_to_period(t["period"]);
            Transaction* new_tran = new RegularExpense(id, amount, date, category_id, account_id, next_date, period);
            manager.add_transaction_internal(new_tran);
        }
        else if (type == "transfer") {
            unsigned destination_id = t["destination_id"];
            Transaction* new_tran = new Transfer(id, amount, date, account_id, destination_id);
            manager.add_transaction_internal(new_tran);
        }
    }
    for (auto& c : j["categories"]) {
        unsigned id = c["id"];
        TransactionType type = t_type_from_string(c["transaction_type"]);
        std::string name = c["name"];
        std::string color = c["color"];
        Category new_cat (id, type, name, color);
        manager.add_category_internal(new_cat);
    }
    for (auto& b: j["budgets"]) {
        unsigned category_id = b["category_id"];
        double limit = b["limit"];
        double current_amount = b["current_amount"];
        const Category cat= manager.get_category_by_id(category_id);
        Budget new_budget(cat, limit, current_amount);
        manager.add_budget_internal(new_budget);
    }
}
