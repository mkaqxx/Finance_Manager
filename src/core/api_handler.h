#pragma once
#include "finance_manager.h"
#include <memory>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

class ApiHandler {
private:
    std::unique_ptr<Finance_manager> owned_manager;
    Finance_manager* manager_ptr;

    Finance_manager& mgr() { return *manager_ptr; }
    const Finance_manager& mgr() const { return *manager_ptr; }

public:
    // Конструктор по умолчанию (создает собственный экземпляр Finance_manager)
    ApiHandler();
    // Конструктор с передачей существующего Finance_manager
    explicit ApiHandler(Finance_manager& manager);
    ~ApiHandler() = default;

    // Инициализация/перезагрузка данных (например, после FS.syncfs в IDBFS)
    void init();

    // Получение данных (возвращают JSON-строки для фронтенда)
    std::string get_accounts();
    std::string get_transactions();
    std::string get_categories();
    std::string get_budgets();
    std::string get_transactions_by_account(unsigned id);
    std::string get_monthly_dashboard();
    std::string get_category_report(const std::string& from, const std::string& to);
    std::string get_yearly_report(int year);

    // Добавление данных (принимают JSON-строку с объектом)
    std::string add_account(const std::string& data_json);
    std::string add_transaction(const std::string& data_json);
    std::string add_category(const std::string& data_json);
    std::string add_budget(const std::string& data_json);

    // Удаление данных
    std::string remove_account(unsigned id);
    std::string remove_transaction(unsigned id);
    std::string remove_category(unsigned id);
    std::string remove_budget(unsigned id);

    // Редактирование данных (принимают JSON-строку с объектом)
    std::string edit_account(const std::string& data_json);
    std::string edit_budget(const std::string& data_json);

    // Прямой доступ к Finance_manager
    Finance_manager& get_finance_manager() { return mgr(); }
};

// Глобальная функция для получения инстанса ApiHandler
ApiHandler* get_wasm_api_handler();