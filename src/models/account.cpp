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

std::string CashAccount::get_info() const { //строка с информацией
    std::stringstream ss;
    ss<<"cash_account\nName: "<<name<<", Balance: "<< balance<<", Currency: "<<CurrencyToString(currency);
    return ss.str();
}


std::string BankAccount::get_info() const { //строка с информацией
    std::stringstream ss;
    ss<<"bank_account\nName: "<<name<<", Balance: "<<balance<<", Currency: "
        <<CurrencyToString(currency)<<", Last four digits: "<<last_four_digits;
    return ss.str();
}


std::string SavingsAccount::get_info() const { //строка с информацией
    std::stringstream ss;
    ss << "savings_account\nName: "<<  name << ", Balance: "<< balance<< ", Currency: "<<CurrencyToString(currency)<<
        ", Goal amount: " << goal_amount << ", Deadline: " << deadline << ", Progress: " << get_progress() <<'%';
    return ss.str();
}


void Account::apply_transaction(Transaction &transaction) {
    if (auto* transfer = dynamic_cast<Transfer*>(&transaction)) { //если перевод
        if (transfer->get_account_id() == id) { //если это отправитель
            transfer->apply_to_source(balance);
        }
        if (transfer->get_destination_id() == id) { //если это получатель
            transfer->apply_to_destination(balance);
        }
    }
    else {//не перевод
        transaction.apply_to_balance(balance);
    }
}


double SavingsAccount::monthly_required() const {
    if (balance >= goal_amount) return 0.0; //уже есть нужная сумма
    //получение текущей даты
    auto today = std::chrono::system_clock::now();
    std::chrono::year_month_day ymd{
        std::chrono::floor<std::chrono::days>(today)};
    //получение остатков срока
    int year_diff = static_cast<int>(deadline.year()) - static_cast<int>(ymd.year());
    int month_diff = static_cast<int>(unsigned(deadline.month())) - static_cast<int>(unsigned(ymd.month()));
    int months_left = year_diff * 12 + month_diff;

    if (months_left <= 0) return std::max(0.0, goal_amount - balance);//если время вышло
    return std::max(0.0, (goal_amount - balance) / months_left);
}



