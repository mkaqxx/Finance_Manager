#include "statistics.h"


double Statistics::total_income(const std::vector<Transaction*> &transactions,
        std::chrono::year_month_day start_date, std::chrono::year_month_day end_date) const;