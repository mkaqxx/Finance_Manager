#pragma once
#include "finance_manager.h"
#include <memory>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#endif

//класс-мост между webassembly / c++ ядром и javascript фронтендом
class ApiHandler {
private:
    std::unique_ptr<Finance_manager> owned_manager; //собственный экземпляр менеджера при автономном запуске
    Finance_manager* manager_ptr; //указатель на рабочий менеджер финансов

    Finance_manager& mgr() { return *manager_ptr; }
    const Finance_manager& mgr() const { return *manager_ptr; }

public:
    //конструктор по умолчанию (создает собственный экземпляр finance_manager)
    ApiHandler();
    explicit ApiHandler(Finance_manager& manager);
    virtual ~ApiHandler(); //виртуальный деструктор

    //инициализация/перезагрузка данных (например, после синхронизации idbfs)
    void init();

    //получение данных (возвращают json-строки для фронтенда)
    std::string get_accounts();
    std::string get_transactions();
    std::string get_categories();
    std::string get_budgets();
    std::string get_transactions_by_account(unsigned id);
    std::string get_monthly_dashboard();
    std::string get_category_report(const std::string& from, const std::string& to);
    std::string get_yearly_report(int year);

    //добавление данных (принимают json-строку с объектом)
    std::string add_account(const std::string& data_json);
    std::string add_transaction(const std::string& data_json);
    std::string add_category(const std::string& data_json);
    std::string add_budget(const std::string& data_json);

    //удаление данных
    std::string remove_account(unsigned id);
    std::string remove_transaction(unsigned id);
    std::string remove_category(unsigned id);
    std::string remove_budget(unsigned id);

    //редактирование данных (принимают json-строку с объектом)
    std::string edit_account(const std::string& data_json);
    std::string edit_budget(const std::string& data_json);

    //прямой доступ к finance_manager
    Finance_manager& get_finance_manager() { return mgr(); }
};

//глобальная функция для получения инстанса apihandler в js
ApiHandler* get_wasm_api_handler();