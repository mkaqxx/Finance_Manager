//мост между js и с++ через webview2 с поддержкой автономного режима
//хранит методы отправки запросов в бэкенд

//демо-данные для автономного режима (браузер / headless / тесты)
const mockDataStore = {
    accounts: [
        { id: 1, name: "Основная карта", balance: 4850.50, currency: "byn", type: "bank_account", number: "4589" },
        { id: 2, name: "Наличные (Кошелек)", balance: 320.00, currency: "byn", type: "cash_account" },
        { id: 3, name: "Копилка на отпуск", balance: 2400.00, currency: "usd", type: "savings_account", goal: 3500.00, deadline: "2026-12-31", progress: 68.6, monthly_required: 550.00 },
        { id: 4, name: "Валютная заначка", balance: 850.00, currency: "eur", type: "cash_account" },
        { id: 5, name: "Резервный фонд", balance: 4500.00, currency: "byn", type: "savings_account", goal: 10000.00, deadline: "2027-06-30", progress: 45.0, monthly_required: 690.00 }
    ],
    categories: [
        { id: 1, name: "Продукты", color: "#10b981", type: "expense" },
        { id: 2, name: "Транспорт", color: "#38bdf8", type: "expense" },
        { id: 3, name: "Кафе и рестораны", color: "#f59e0b", type: "expense" },
        { id: 4, name: "Зарплата", color: "#10b981", type: "income" },
        { id: 5, name: "Инвестиции", color: "#8b5cf6", type: "income" },
        { id: 6, name: "Развлечения и кино", color: "#ec4899", type: "expense" },
        { id: 7, name: "Подписки", color: "#6366f1", type: "expense" },
        { id: 8, name: "Здоровье и аптеки", color: "#ef4444", type: "expense" },
        { id: 9, name: "Коммунальные услуги", color: "#14b8a6", type: "expense" },
        { id: 10, name: "Одежда и обувь", color: "#a855f7", type: "expense" }
    ],
    budgets: [
        { category_id: 1, name: "Продукты", limit: 1200.00, current_amount: 820.00, status: "normal" },
        { category_id: 2, name: "Транспорт", limit: 250.00, current_amount: 235.00, status: "warning" },
        { category_id: 3, name: "Кафе и рестораны", limit: 400.00, current_amount: 460.00, status: "exceeded" },
        { category_id: 7, name: "Подписки", limit: 90.00, current_amount: 65.00, status: "normal" }
    ],
    transactions: [
        { id: 1, amount: 85.50, date: "2026-11-06", type: "expense", account_id: 1, category_id: 1, currency: "byn" },
        { id: 2, amount: 45.00, date: "2026-11-05", type: "expense", account_id: 1, category_id: 3, currency: "byn" },
        { id: 3, amount: 2800.00, date: "2026-11-04", type: "income", account_id: 1, category_id: 4, currency: "byn" },
        { id: 4, amount: 25.00, date: "2026-11-03", type: "expense", account_id: 1, category_id: 2, currency: "byn" },
        { id: 5, amount: 15.00, date: "2026-11-02", type: "expense", account_id: 1, category_id: 7, currency: "byn" },
        { id: 6, amount: 300.00, date: "2026-11-01", type: "transfer", account_id: 1, destination_id: 2, category_id: 0, currency: "byn" },
        { id: 7, amount: 500.00, date: "2026-10-28", type: "income", account_id: 3, category_id: 5, currency: "usd" },
        { id: 8, amount: 110.00, date: "2026-10-20", type: "expense", account_id: 1, category_id: 6, currency: "byn" }
    ]
};

const windowAPI = {
    //универсальный метод вызова с++ биндинга
    async fetch(method, data = null) {
        // Если функция привязана в C++ через WebView2
        if (typeof window[method] === 'function') {
            try {
                const rawResponse = data !== null ? await window[method](data) : await window[method]();
                const response = typeof rawResponse === 'string' ? JSON.parse(rawResponse) : rawResponse;
                if (response && response.error) throw new Error(response.error);
                return response;
            } catch (error) {
                console.error(`Ошибка в API (${method}):`, error);
                throw error;
            }
        }

        // Автономный fallback для предпросмотра и тестирования
        return this.mockHandler(method, data);
    },

    mockHandler(method, data) {
        switch (method) {
            case 'getAccounts':
                return Promise.resolve(JSON.parse(JSON.stringify(mockDataStore.accounts)));
            case 'getCategories':
                return Promise.resolve(JSON.parse(JSON.stringify(mockDataStore.categories)));
            case 'getBudgets':
                return Promise.resolve(JSON.parse(JSON.stringify(mockDataStore.budgets)));
            case 'getTransactions':
                return Promise.resolve(JSON.parse(JSON.stringify(mockDataStore.transactions)));
            case 'getMonthlyDashboard': {
                return Promise.resolve({
                    byn: {
                        balance: 5170.50,
                        expense: 1580.00,
                        income: 3650.00,
                        top_categories: [
                            { name: "Продукты", amount: 820.00, color: "#10b981" },
                            { name: "Кафе и рестораны", amount: 460.00, color: "#f59e0b" },
                            { name: "Транспорт", amount: 235.00, color: "#38bdf8" },
                            { name: "Подписки", amount: 65.00, color: "#6366f1" }
                        ]
                    },
                    usd: {
                        balance: 2400.00,
                        expense: 120.00,
                        income: 500.00,
                        top_categories: [
                            { name: "Подписки", amount: 120.00, color: "#6366f1" }
                        ]
                    },
                    eur: {
                        balance: 850.00,
                        expense: 0.00,
                        income: 0.00,
                        top_categories: []
                    },
                    rub: {
                        balance: 0.00,
                        expense: 0.00,
                        income: 0.00,
                        top_categories: []
                    }
                });
            }
            case 'getTransactionsByAccount': {
                const accId = Array.isArray(data) ? data[0] : (data?.accountId || data);
                const filtered = mockDataStore.transactions.filter(t => t.account_id === accId || t.destination_id === accId);
                return Promise.resolve(filtered);
            }
            case 'getCategoryReport': {
                return Promise.resolve({
                    byn: [
                        { name: "Продукты", amount: 820.00, color: "#10b981", percent: 36.3 },
                        { name: "Кафе и рестораны", amount: 460.00, color: "#f59e0b", percent: 20.4 },
                        { name: "Транспорт", amount: 235.00, color: "#38bdf8", percent: 10.4 },
                        { name: "Развлечения и кино", amount: 210.00, color: "#ec4899", percent: 9.3 },
                        { name: "Здоровье и аптеки", amount: 180.00, color: "#ef4444", percent: 8.0 },
                        { name: "Коммунальные услуги", amount: 150.00, color: "#14b8a6", percent: 6.6 },
                        { name: "Одежда и обувь", amount: 135.00, color: "#a855f7", percent: 6.0 },
                        { name: "Подписки", amount: 65.00, color: "#6366f1", percent: 2.9 }
                    ],
                    usd: [
                        { name: "Подписки", amount: 120.00, color: "#6366f1", percent: 100.0 }
                    ],
                    eur: [],
                    rub: []
                });
            }
            case 'getYearlyReport': {
                const mkMonths = (incomes, expenses) => {
                    return Array.from({ length: 12 }, (_, i) => ({
                        month: i + 1,
                        income: incomes[i] || 0,
                        expense: expenses[i] || 0
                    }));
                };
                return Promise.resolve({
                    byn: mkMonths(
                        [2200, 2400, 2500, 2600, 3100, 2900, 3000, 3200, 3400, 3500, 3650, 0],
                        [1400, 1550, 1600, 1450, 1800, 1750, 1900, 1600, 1700, 1850, 1580, 0]
                    ),
                    usd: mkMonths(
                        [500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 0],
                        [120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 0]
                    ),
                    eur: mkMonths([], []),
                    rub: mkMonths([], [])
                });
            }
            case 'addAccount': {
                const newAcc = { id: Date.now(), ...data, number: data.last_four_digits || '' };
                mockDataStore.accounts.push(newAcc);
                return Promise.resolve({ success: true, id: newAcc.id });
            }
            case 'editAccount': {
                const idx = mockDataStore.accounts.findIndex(a => a.id === data.id);
                if (idx !== -1) {
                    mockDataStore.accounts[idx] = { ...mockDataStore.accounts[idx], ...data };
                }
                return Promise.resolve({ success: true });
            }
            case 'removeAccount': {
                const id = Array.isArray(data) ? data[0] : data;
                mockDataStore.accounts = mockDataStore.accounts.filter(a => a.id !== id);
                return Promise.resolve({ success: true });
            }
            case 'addTransaction': {
                const newTx = { id: Date.now(), currency: 'byn', ...data };
                mockDataStore.transactions.unshift(newTx);
                return Promise.resolve({ success: true, id: newTx.id });
            }
            case 'removeTransaction': {
                const id = Array.isArray(data) ? data[0] : data;
                mockDataStore.transactions = mockDataStore.transactions.filter(t => t.id !== id);
                return Promise.resolve({ success: true });
            }
            case 'addCategory': {
                const newCat = { id: Date.now(), ...data };
                mockDataStore.categories.push(newCat);
                return Promise.resolve({ success: true, id: newCat.id });
            }
            case 'removeCategory': {
                const id = Array.isArray(data) ? data[0] : data;
                mockDataStore.categories = mockDataStore.categories.filter(c => c.id !== id);
                return Promise.resolve({ success: true });
            }
            case 'addBudget': {
                const cat = mockDataStore.categories.find(c => c.id === data.category_id);
                const newBud = { category_id: data.category_id, name: cat ? cat.name : "Лимит", limit: data.limit, current_amount: 0, status: "normal" };
                mockDataStore.budgets.push(newBud);
                return Promise.resolve({ success: true });
            }
            case 'editBudget': {
                const idx = mockDataStore.budgets.findIndex(b => b.category_id === data.category_id);
                if (idx !== -1) mockDataStore.budgets[idx].limit = data.limit;
                return Promise.resolve({ success: true });
            }
            case 'removeBudget': {
                const id = Array.isArray(data) ? data[0] : data;
                mockDataStore.budgets = mockDataStore.budgets.filter(b => b.category_id !== id);
                return Promise.resolve({ success: true });
            }
            default:
                return Promise.resolve({});
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