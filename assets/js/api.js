const windowAPI = {
    async fetch(method, data = null) {
        try {
            const rawResponse = data ? await window[method](data) : await window[method]();
            const response = typeof rawResponse === 'string' ? JSON.parse(rawResponse) : rawResponse;
            if (response && response.error) throw new Error(response.error);
            return response;
        } catch (error) {
            console.error(`Ошибка в API (${method}):`, error);
            throw error;
        }
    },
    getAccounts: () => windowAPI.fetch('getAccounts'),
    getTransactions: () => windowAPI.fetch('getTransactions'),
    addAccount: (data) => windowAPI.fetch('addAccount', [data]),
    addTransaction: (data) => windowAPI.fetch('addTransaction', [data]),
    getCategories: () => windowAPI.fetch('getCategories'),
    getBudgets: () => windowAPI.fetch('getBudgets'),
    addCategory: (data) => windowAPI.fetch('addCategory', [data]),
    addBudget: (data) => windowAPI.fetch('addBudget', [data]),
    getMonthlyDashboard: () => windowAPI.fetch('getMonthlyDashboard'),
    getCategoryReport: (from, to) => windowAPI.fetch('getCategoryReport', [[from, to]]),
    getYearlyReport: (year) => windowAPI.fetch('getYearlyReport', [[year]])
};