#include "budget.h"


std::string t_type_to_string(const TransactionType &type) {
    switch (type) {
        case TransactionType::EXPENSE: return "expense";
        case TransactionType::INCOME:   return "income";
    }
}


TransactionType t_type_from_string(const std::string &type) {
    if ( type == "expense" ) return TransactionType::EXPENSE;
    else return TransactionType::INCOME;
}

std::string status_to_string(const Status &status) {
    switch (status) {
        case Status::NORMAL: return "normal";
        case Status::WARNING: return "warning";
        case Status::EXCEEDED: return "exceeded";
    }
}


Status status_from_string(const std::string &status) {
    if ( status == "normal" ) return Status::NORMAL;
    else if ( status == "warning" ) return Status::WARNING;
    else return Status::EXCEEDED;
}


Status Budget::get_status() const {
    if ( current_amount/limit >= 1.0 ) return Status::EXCEEDED;
    if ( current_amount/limit >= 0.8 ) return Status::WARNING;
    else return Status::NORMAL;
}
