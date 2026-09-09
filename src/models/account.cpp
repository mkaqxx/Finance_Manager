#include "account.h"


std::string CurrencyToString(Currency currency) {
    switch (currency) {
        case Currency::BYN: return "byn";
        case Currency::USD: return "usd";
        case Currency::EUR: return "eur";
        case Currency::RUB: return "rub";
        default:  throw std::runtime_error("Unknown currency");
    }
}


Currency StringToCurrency(const std::string &currency) {
    if (currency == "usd") return Currency::USD;
    else if (currency == "eur") return Currency::EUR;
    else if (currency == "rub") return Currency::RUB;
    else if (currency == "byn") return Currency::BYN;
    else throw std::runtime_error("Unknown currency: " + currency);
}

std::string CashAccount::get_info() const {
    std::stringstream ss;
    ss<<"cash_account\nName: "<<name<<", Balance: "<< balance<<", Currency: "<<CurrencyToString(currency);
    return ss.str();
}


std::string BankAccount::get_info() const {
    std::stringstream ss;
    ss<<"bank_account\nName: "<<name<<", Balance: "<<balance<<", Currency: "
        <<CurrencyToString(currency)<<", Last four digits: "<<last_four_digits;
    return ss.str();
}


std::string SavingsAccount::get_info() const {
    std::stringstream ss;
    ss << "savings_account\nName: "<<  name << ", Balance: "<< balance<< ", Currency: "<<CurrencyToString(currency)<<
        ", Goal amount: " << goal_amount << ", Deadline: " << deadline << ", Progress: " << get_progress() <<'%';
    return ss.str();
}


void Account::apply_transaction(Transaction &transaction) {
    if (auto* transfer = dynamic_cast<Transfer*>(&transaction)) {
        if (transfer->get_account_id() == id) {
            transfer->apply_to_source(balance);
        }
        else if (transfer->get_destination_id() == id) {
            transfer->apply_to_destination(balance);
        }
    }
    else {
        // Для всех остальных типов транзакций
        transaction.apply_to_balance(balance);
    }
}


double SavingsAccount::monthly_required() const {
    auto today = std::chrono::system_clock::now();
    std::chrono::year_month_day ymd{
        std::chrono::floor<std::chrono::days>(today)};
    int months_left =
        (static_cast<int>(deadline.year()) - static_cast<int>(ymd.year())) * 12
        + (static_cast<unsigned>(deadline.month()) - static_cast<unsigned>(ymd.month()));
    if (months_left <= 0) return goal_amount - balance;
    return (goal_amount - balance) / months_left;
}
