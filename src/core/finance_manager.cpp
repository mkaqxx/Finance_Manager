#include "finance_manager.h"
#include <algorithm>


//конструктор. загружает данные из файла
//создает хэш таблицы, проверяет регулярные расходы
Finance_manager::Finance_manager() {
    //явно указывается возвращаемое значение void от std::expected
    (void)storage.load(accounts, transactions, categories, budgets);
    rebuild_lookups();
    (void)check_the_regular_expense_date();
}


//деструктор. сохраняет данные. не вызывает исключения
Finance_manager::~Finance_manager() noexcept {
    (void)save_data();
}


//обновление хэш таблиц
void Finance_manager::rebuild_lookups() {
    //очистка старых хэш таблиц
    account_lookup.clear();
    //цикл по счетам
    for (const auto& a : accounts) {
        //если счет существует, добавляем его в таблицу по его айди
        if (a) account_lookup[a->get_id()] = a.get();
    }
    //соответственно для категорий
    category_lookup.clear();
    for (size_t i = 0; i < categories.size(); ++i) {
        category_lookup[categories[i].id] = i;
    }
}


//сохранение данных
std::expected<void, AppError> Finance_manager::save_data() noexcept {
    //если никаких изменений не было
    if (!is_dirty) return {};
    //сохранение
    auto res = storage.save(accounts, transactions, categories, budgets);
    //обнуление флага при удачном сохранении
    if (res) is_dirty = false;
    return res;
}


//возвращает вектор сырых указателей на счета
const std::vector<Account*> Finance_manager::get_accounts_raw() const {
    //вектор указателей на счет
    std::vector<Account*> res;
    //выделение памяти под accounts.size() элементов
    res.reserve(accounts.size());
    //перенос каждого счета в новый массив
    for (const auto& a : accounts) res.push_back(a.get());
    return res;
}


//возвращает вектор сырых указателей на транзакции
const std::vector<Transaction*> Finance_manager::get_transactions_raw() const {
    //вектор указателей на транзакции
    std::vector<Transaction*> res;
    //выделение памяти под transactions.size() элементов
    res.reserve(transactions.size());
    //перенос транзакций в новый массив
    for (const auto& t : transactions) res.push_back(t.get());
    return res;
}


//добавление счета
std::expected<void, AppError> Finance_manager::add_account(std::unique_ptr<Account> account) {
    //проверка на nullptr
    if (!account) return std::unexpected(AppError{ErrorCode::InvalidInput, "Передан пустой счёт"});
    //добавление счета в хэш-таблицу
    account_lookup[account->get_id()] = account.get();
    //перенос счета в массив
    accounts.push_back(std::move(account));
    //сброс флага и сохранение данных
    is_dirty = true;
    return save_data();
}


//добавление транзакции
std::expected<void, AppError> Finance_manager::add_transaction(std::unique_ptr<Transaction> transaction) {
    //проверка на nullptr
    if (!transaction) return std::unexpected(AppError{ErrorCode::InvalidInput, "Передана пустая транзакция"});
    //получение сырого указателя для dynamic_cast
    Transaction* raw_tx = transaction.get();

    //обработка перевода между счетами
    if (auto* trans = dynamic_cast<Transfer *>(raw_tx)) {
        //поиск счетов списания и зачисления через хэш-таблицу
        Account* src = get_account_by_id(trans->get_account_id());
        Account* dest = get_account_by_id(trans->get_destination_id());
        if (src == dest) {
            if (src) src->apply_transaction(*trans);
        }
        else {
            if (src) src->apply_transaction(*trans);
            if (dest) dest->apply_transaction(*trans);
        }
    }
    //обработка дохода или расхода
    else {
        //поиск счета через хэш-таблицу
        Account* acc = get_account_by_id(raw_tx->get_account_id());
        if (acc) {
            //применение транзакции к счету
            acc->apply_transaction(*raw_tx);
        }
        //обновление лимита бюджета при расходе в валюте byn
        std::string type = raw_tx->get_type();
        if (type == "expense" || type == "regular_expense") {
            if (acc && acc->get_currency() == Currency::BYN) {
                unsigned category_id = raw_tx->get_category_id();
                //обновление суммы в бюджете соответствующей категории
                for (auto& budget : budgets) {
                    if (budget.get_category().id == category_id) {
                        budget.update_amount(raw_tx->get_amount());
                    }
                }
            }
        }
    }
    //добавление транзакции в массив и сохранение
    transactions.push_back(std::move(transaction));
    is_dirty = true;
    return save_data();
}


//добавление категории
std::expected<void, AppError> Finance_manager::add_category(const Category& category) {
    //добавление в массив и хэш-таблицу
    categories.push_back(category);
    category_lookup[category.id] = categories.size() - 1;
    //сохранение
    is_dirty = true;
    return save_data();
}


//добавление лимита
std::expected<void, AppError> Finance_manager::add_budget(const Budget& budget) {
    budgets.push_back(budget);
    is_dirty = true;
    return save_data();
}


//поиск категории по айди
const Category & Finance_manager::get_category_by_id(unsigned id) const {
    //получение итератора категории
    auto it = category_lookup.find(id);
    if (it != category_lookup.end() && it->second < categories.size()) {
        return categories[it->second];
    }
    throw std::runtime_error("Category not found: " + std::to_string(id));
}


//поиск счета по айди
Account *Finance_manager::get_account_by_id(unsigned id) const {
    //получение итератора счета
    auto it = account_lookup.find(id);
    return (it != account_lookup.end()) ? it->second : nullptr;
}
    

//проверка даты регулярных расходов
std::expected<void, AppError> Finance_manager::check_the_regular_expense_date() {
    //получение текущей точки времени
    auto now = std::chrono::system_clock::now();
    std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
    //вектор для добавленных операций
    std::vector<std::unique_ptr<Transaction>> to_add;

    //генерация нового уникального айди
    unsigned max_id = 0;
    for (const auto& t : transactions) {
        if (t->get_id() > max_id) max_id = t->get_id();
    }

    for (auto& transaction : transactions) {
        if (auto regular = dynamic_cast<RegularExpense *>(transaction.get())) {
            //цикл, пока дата не дойдет до текущей
            while (regular->get_next_date() <= ymd) {
                unsigned new_id = ++max_id;
                //создание новой транзакции и перенос ее в вектор для добавления
                auto new_expense = std::make_unique<Expense>(new_id, regular->get_amount(), regular->get_next_date(),
                    regular->get_category_id(), regular->get_account_id());
                to_add.push_back(std::move(new_expense));
                //обновление даты
                regular->update_next_date();
                is_dirty = true;
            }
        }
    }
    //добавление каждой транзакции из массива
    for (auto& t : to_add) {
        auto res = add_transaction(std::move(t));
        if (!res) {
            return std::unexpected(res.error());
        }
    }
    //сохранение
    if (is_dirty && to_add.empty()) {
        return save_data();
    }
    return {};
}


//получение месячного отчета
json Finance_manager::get_monthly_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const {
    //получение сырых указателей
    auto txs = get_transactions_raw();
    auto accs = get_accounts_raw();
    //создание класса-отчета и генерация
    MonthlyReport report(txs, accs, categories, from, to);
    return report.generate();
}


//получение отчета по категориям
json Finance_manager::get_category_report(std::chrono::year_month_day from, std::chrono::year_month_day to) const {
    //получение сырых указателей
    auto txs = get_transactions_raw();
    auto accs = get_accounts_raw();
    //создание класса-отчета и генерация
    CategoryReport report(txs, accs, categories, from, to);
    return report.generate();
}


//получение годового отчета
json Finance_manager::get_yearly_report(int year) const {
    //получение сырых указателей
    auto txs = get_transactions_raw();
    auto accs = get_accounts_raw();
    //создание класса-отчета и генерация
    YearlyReport report(txs, accs, year);
    return report.generate();
}


//удаление счета
std::expected<void, AppError> Finance_manager::remove_account(unsigned id) {
    //поиск нужного счета
    auto it_acc = std::find_if(accounts.begin(), accounts.end(), [id](const auto& a) {
        return a->get_id() == id;
    });
    if (it_acc == accounts.end()) {
        return std::unexpected(AppError{ErrorCode::NotFound, "Счёт с ID " + std::to_string(id) + " не найден"});
    }

    //сохранение валюты
    const Currency acc_currency = (*it_acc)->get_currency();

    //откат транзакций счета
    for (const auto& t : transactions) {
        if (auto* trans = dynamic_cast<Transfer*>(t.get())) {
            if (trans->get_account_id() == id && trans->get_destination_id() != id) {
                if (Account* dest = get_account_by_id(trans->get_destination_id())) {
                    //создание обратной транзакции
                    Expense rollback(0, trans->get_amount(), trans->get_date(), 0, 0);
                    dest->apply_transaction(rollback);
                }
            }
            else if (trans->get_destination_id() == id && trans->get_account_id() != id) {
                if (Account* src = get_account_by_id(trans->get_account_id())) {
                    //создание обратной транзакции
                    Income rollback(0, trans->get_amount(), trans->get_date(), 0, 0);
                    src->apply_transaction(rollback);
                }
            }
        }
        //откат лимитов
        else if (t->get_account_id() == id) {
            if (t->get_type() == "expense" || t->get_type() == "regular_expense") {
                if (acc_currency == Currency::BYN) {
                    for (auto& budget : budgets) {
                        if (budget.get_category().id == t->get_category_id()) {
                            budget.update_amount(-t->get_amount());
                        }
                    }
                }
            }
        }
    }
    //удаление связанных транзакций счета из вектора
    std::erase_if(transactions, [id](const auto& t) {
        if (t->get_account_id() == id) return true;
        if (auto* trans = dynamic_cast<Transfer*>(t.get())) {
            if (trans->get_destination_id() == id) return true;
        }
        return false;
    });

    account_lookup.erase(id);
    accounts.erase(it_acc);

    //сохранение
    is_dirty = true;
    return save_data();
}


//удаление категории
std::expected<void, AppError> Finance_manager::remove_category(unsigned id) {
    //проверка существования категории
    auto cat_it = std::find_if(categories.begin(), categories.end(), [id](const auto& c) {
        return c.id == id;
    });
    if (cat_it == categories.end()) {
        return std::unexpected(AppError{ErrorCode::NotFound, "Категория с ID " + std::to_string(id) + " не найдена"});
    }

    //откат соответствующих транзакций
    for (const auto& t : transactions) {
        if (t->get_type() != "transfer" && t->get_category_id() == id) {
            if (Account* acc = get_account_by_id(t->get_account_id())) {
                if (t->get_type() == "expense" || t->get_type() == "regular_expense") {
                    Income rev(0, t->get_amount(), t->get_date(), 0, 0);
                    acc->apply_transaction(rev);
                } else if (t->get_type() == "income") {
                    Expense rev(0, t->get_amount(), t->get_date(), 0, 0);
                    acc->apply_transaction(rev);
                }
            }
        }
    }

    //удаление соответствующих транзакций
    std::erase_if(transactions, [id](const auto& t) {
        return t->get_type() != "transfer" && t->get_category_id() == id;
    });

    //удаление соответствующего лимита
    std::erase_if(budgets, [id](const auto& b) {
        return b.get_category().id == id;
    });

    //удаление категории
    std::erase_if(categories, [id](const auto& c) {
        return c.id == id;
    });


    //очистка словаря
    category_lookup.clear();
    for (size_t i = 0; i < categories.size(); ++i) {
        category_lookup[categories[i].id] = i;
    }

    //сохранение
    is_dirty = true;
    return save_data();
}


//удаление лимита
std::expected<void, AppError> Finance_manager::remove_budget(unsigned id) {
    //поиск лимита по id категории
    auto it = std::find_if(budgets.begin(), budgets.end(), [id](const auto& b) {
        return b.get_category().id == id;
    });
    if (it == budgets.end()) {
        return std::unexpected(AppError{ErrorCode::NotFound, "Лимит бюджета для категории с ID " + std::to_string(id) + " не найден"});
    }
    budgets.erase(it);
    is_dirty = true;
    return save_data();
}


//удаление транзакции
std::expected<void, AppError> Finance_manager::remove_transaction(unsigned id) {
    bool found = false; //флаг найдена ли транзакция
    //цикл по вектору транзакций
    for (auto it = transactions.begin(); it != transactions.end(); ++it) {
        if ((*it)->get_id() == id) {
            Transaction* tx = it->get();
            //если перевод
            if (auto* transfer = dynamic_cast<Transfer*>(tx)) {
                //получение счетов для отката перевода
                Account* src = get_account_by_id(transfer->get_account_id());
                Account* dest = get_account_by_id(transfer->get_destination_id());
                //проверка наличия счетов перед откатом
                if (!src || !dest) {
                    return std::unexpected(AppError{ErrorCode::NotFound, "Счета для отката перевода не найдены"});
                }
                Income reversal_income(0, tx->get_amount(), tx->get_date(), 0, 0);
                src->apply_transaction(reversal_income);
                Expense reversal_expense(0, tx->get_amount(), tx->get_date(), 0, 0);
                dest->apply_transaction(reversal_expense);
            }
            else {
                //получение счета для отката
                Account* acc = get_account_by_id(tx->get_account_id());
                if (!acc) {
                    return std::unexpected(AppError{ErrorCode::NotFound, "Счет для отката транзакции не найден"});
                }
                //откат транзакции и лимитов
                if (tx->get_type() == "expense" || tx->get_type() == "regular_expense") {
                    Income reversal_income(0, tx->get_amount(), tx->get_date(), 0, 0);
                    acc->apply_transaction(reversal_income);
                    if (acc->get_currency() == Currency::BYN) {
                        for (auto& budget : budgets) {
                            if (budget.get_category().id == tx->get_category_id()) {
                                budget.update_amount(-tx->get_amount());
                            }
                        }
                    }
                }
                else if (tx->get_type() == "income") {
                    Expense reversal_expense(0, tx->get_amount(), tx->get_date(), 0, 0);
                    acc->apply_transaction(reversal_expense);
                }
            }
            //удаление транзакции
            transactions.erase(it);
            found = true;
            break;
        }
    }
    //если транзакция не найдена
    if (!found) {
        return std::unexpected(AppError{ErrorCode::NotFound, "Транзакция с ID " + std::to_string(id) + " не найдена"});
    }
    //сохранение
    is_dirty = true;
    return save_data();
}


//редактирование лимита
std::expected<void, AppError> Finance_manager::edit_budget(unsigned id, double new_limit) {
    bool found = false;
    //поиск лимита и его изменение
    for (auto &budget : budgets) {
        if (budget.get_category().id == id) {
            budget.set_limit(new_limit);
            found = true;
            break;
        }
    }
    if (!found) {
        return std::unexpected(AppError{ErrorCode::NotFound, "Бюджет для категории " + std::to_string(id) + " не найден"});
    }
    is_dirty = true;
    return save_data();
}


//редактирование счета
std::expected<void, AppError> Finance_manager::edit_account(unsigned id, const std::string& name, double balance, Currency currency,
                                                            const std::string& last_four_digits, double goal_amount,
                                                            std::chrono::year_month_day deadline) {
    //получение счета
    Account* edit_acc = get_account_by_id(id);
    if (!edit_acc) {
        return std::unexpected(AppError{ErrorCode::NotFound, "Счёт с ID " + std::to_string(id) + " не найден"});
    }
    //установка новых базовых параметров
    edit_acc->set_name(name);
    edit_acc->set_balance(balance);
    edit_acc->set_currency(currency);
    //установка новых уникальных параметров
    if (auto* edit_bank = dynamic_cast<BankAccount*>(edit_acc)) {
        edit_bank->set_last_four_digits(last_four_digits);
    } else if (auto* edit_savings = dynamic_cast<SavingsAccount*>(edit_acc)) {
        edit_savings->set_goal_amount(goal_amount);
        edit_savings->set_deadline(deadline);
    }
    //сохранение
    is_dirty = true;
    return save_data();
}
