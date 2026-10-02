let wasmApi = null;

async function initFinanceWasm() {
    // 1. Инициализируем модуль Emscripten
    const Module = await createFinanceManagerModule();
    const FS = Module.FS;
    const IDBFS = Module.IDBFS;

    // 2. Создаем точку монтирования и монтируем IndexedDB
    try {
        FS.mkdir('/data');
    } catch (e) {
        // папка уже может существовать
    }
    FS.mount(IDBFS, {}, '/data');

    // 3. Синхронизируем из IndexedDB в виртуальную память (true = из IDB в MEMFS)
    await new Promise((resolve, reject) => {
        FS.syncfs(true, (err) => {
            if (err) {
                console.error('Ошибка загрузки базы IDBFS:', err);
                reject(err);
            } else {
                console.log('IDBFS успешно синхронизирован с виртуальной ФС.');
                resolve();
            }
        });
    });

    // 4. Получаем инстанс API и загружаем прочитанные данные
    wasmApi = Module.getApiHandler();
    wasmApi.init();

    // 5. Привязываем нативные методы к глобальному объекту window для прозрачной работы api.js:
    window.getAccounts = () => wasmApi.getAccounts();
    window.getTransactions = () => wasmApi.getTransactions();
    window.getCategories = () => wasmApi.getCategories();
    window.getBudgets = () => wasmApi.getBudgets();
    window.getMonthlyDashboard = () => wasmApi.getMonthlyDashboard();

    window.getCategoryReport = (args) => wasmApi.getCategoryReport(args[0], args[1]);
    window.getYearlyReport = (args) => wasmApi.getYearlyReport(args[0]);
    window.getTransactionsByAccount = (args) => wasmApi.getTransactionsByAccount(args[0]);

    window.addAccount = (data) => wasmApi.addAccount(typeof data === 'string' ? data : JSON.stringify(data));
    window.addTransaction = (data) => wasmApi.addTransaction(typeof data === 'string' ? data : JSON.stringify(data));
    window.addCategory = (data) => wasmApi.addCategory(typeof data === 'string' ? data : JSON.stringify(data));
    window.addBudget = (data) => wasmApi.addBudget(typeof data === 'string' ? data : JSON.stringify(data));

    window.removeAccount = (args) => wasmApi.removeAccount(Array.isArray(args) ? args[0] : args);
    window.removeTransaction = (args) => wasmApi.removeTransaction(Array.isArray(args) ? args[0] : args);
    window.removeCategory = (args) => wasmApi.removeCategory(Array.isArray(args) ? args[0] : args);
    window.removeBudget = (args) => wasmApi.removeBudget(Array.isArray(args) ? args[0] : args);

    window.editAccount = (data) => wasmApi.editAccount(typeof data === 'string' ? data : JSON.stringify(data));
    window.editBudget = (data) => wasmApi.editBudget(typeof data === 'string' ? data : JSON.stringify(data));

    // Сигнализируем UI об успешной инициализации
    window.wasmApiReady = true;
    window.dispatchEvent(new Event('finance-wasm-ready'));
}

// Запускаем инициализацию
initFinanceWasm();
