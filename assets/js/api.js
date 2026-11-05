//мост между js и с++ (работает как с webview2, так и с webassembly)
//хранит методы отправки запросов в бэкенд
const windowAPI = {
    //универсальный метод вызова с++ биндинга
    //method - имя функции, привязанной к window
    //data - передаваемые параметры (объект или массив аргументов)
    async fetch(method, data = null) {
        try {
            //вызов функции с++ через глобальный объект window
            //await ждет ответа с++ без блокировки интерфейса
            const rawResponse = data ? await window[method](data) : await window[method]();

            //с++ возвращает строку json, парсим ее в js-объект
            const response = typeof rawResponse === 'string' ? JSON.parse(rawResponse) : rawResponse;

            //если с++ вернул ошибку, выбрасываем исключение
            if (response && response.error) throw new Error(response.error);

            //возврат готовых данных
            return response;
        } catch (error) {
            //логирование ошибки вызова в консоль
            console.error(`Ошибка в API (${method}):`, error);
            //проброс ошибки для обработки в интерфейсе
            throw error;
        }
    },

    //получение списка всех счетов
    getAccounts: () => windowAPI.fetch('getAccounts'),

    //получение всех транзакций
    getTransactions: () => windowAPI.fetch('getTransactions'),

    //получение всех категорий
    getCategories: () => windowAPI.fetch('getCategories'),

    //получение бюджетов и лимитов
    getBudgets: () => windowAPI.fetch('getBudgets'),

    //получение сводки для дашборда за текущий месяц
    getMonthlyDashboard: () => windowAPI.fetch('getMonthlyDashboard'),

    //получение отчета по категориям за период дат
    getCategoryReport: (from, to) => windowAPI.fetch('getCategoryReport', [from, to]),

    //получение годового отчета по месяцам
    getYearlyReport: (year) => windowAPI.fetch('getYearlyReport', [year]),

    //получение истории транзакций конкретного счета
    getTransactionsByAccount: (accountId) => windowAPI.fetch('getTransactionsByAccount', [accountId]),

    //добавление нового счета
    addAccount: (data) => windowAPI.fetch('addAccount', data),

    //добавление новой транзакции
    addTransaction: (data) => windowAPI.fetch('addTransaction', data),

    //добавление новой категории
    addCategory: (data) => windowAPI.fetch('addCategory', data),

    //установка лимита бюджета на категорию
    addBudget: (data) => windowAPI.fetch('addBudget', data),

    //удаление счета по айди
    removeAccount: (accountId) => windowAPI.fetch('removeAccount', [accountId]),

    //удаление транзакции по айди
    removeTransaction: (transactionId) => windowAPI.fetch('removeTransaction', [transactionId]),

    //удаление категории по айди
    removeCategory: (categoryId) => windowAPI.fetch('removeCategory', [categoryId]),

    //удаление бюджета категории
    removeBudget: (categoryId) => windowAPI.fetch('removeBudget', [categoryId]),

    //редактирование счета
    editAccount: (data) => windowAPI.fetch('editAccount', data),

    //изменение лимита бюджета
    editBudget: (data) => windowAPI.fetch("editBudget", data)
};