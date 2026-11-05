//модуль загрузки webassembly ядра и синхронизации с indexeddb
//хранит ссылку на c++ объект ApiHandler
let wasmApi = null;

//асинхронная инициализация wasm модуля
async function initFinanceWasm() {
    //инициализация модуля emscripten и указание пути к .wasm бинарнику
    const Module = await createFinanceManagerModule({
        locateFile: (path) => 'assets/wasm/' + path
    });
    //доступ к виртуальной файловой системе emscripten
    const FS = Module.FS;
    const IDBFS = Module.IDBFS;

    //создание точки монтирования в виртуальной фс
    try {
        FS.mkdir('/data');
    } catch (e) {
        //папка уже может существовать при повторной загрузке
    }
    //монтирование постоянного хранилища indexeddb в папку /data
    FS.mount(IDBFS, {}, '/data');

    //синхронизация данных из indexeddb браузера в виртуальную память (true = из idb в memfs)
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

    //получение c++ объекта api и загрузка считанных данных
    wasmApi = Module.getApiHandler();
    wasmApi.init();

    //привязка нативных методов с++ к объекту window для совместимости с api.js
    //методы чтения
    window.getAccounts = () => wasmApi.getAccounts();
    window.getTransactions = () => wasmApi.getTransactions();
    window.getCategories = () => wasmApi.getCategories();
    window.getBudgets = () => wasmApi.getBudgets();
    window.getMonthlyDashboard = () => wasmApi.getMonthlyDashboard();

    //методы с аргументами дат и айди
    window.getCategoryReport = (args) => wasmApi.getCategoryReport(args[0], args[1]);
    window.getYearlyReport = (args) => wasmApi.getYearlyReport(args[0]);
    window.getTransactionsByAccount = (args) => wasmApi.getTransactionsByAccount(args[0]);

    //методы добавления (передают json-строку в с++)
    window.addAccount = (data) => wasmApi.addAccount(typeof data === 'string' ? data : JSON.stringify(data));
    window.addTransaction = (data) => wasmApi.addTransaction(typeof data === 'string' ? data : JSON.stringify(data));
    window.addCategory = (data) => wasmApi.addCategory(typeof data === 'string' ? data : JSON.stringify(data));
    window.addBudget = (data) => wasmApi.addBudget(typeof data === 'string' ? data : JSON.stringify(data));

    //методы удаления
    window.removeAccount = (args) => wasmApi.removeAccount(Array.isArray(args) ? args[0] : args);
    window.removeTransaction = (args) => wasmApi.removeTransaction(Array.isArray(args) ? args[0] : args);
    window.removeCategory = (args) => wasmApi.removeCategory(Array.isArray(args) ? args[0] : args);
    window.removeBudget = (args) => wasmApi.removeBudget(Array.isArray(args) ? args[0] : args);

    //методы редактирования
    window.editAccount = (data) => wasmApi.editAccount(typeof data === 'string' ? data : JSON.stringify(data));
    window.editBudget = (data) => wasmApi.editBudget(typeof data === 'string' ? data : JSON.stringify(data));

    //сигнал об успешной загрузке wasm ядра для app.js
    window.wasmApiReady = true;
    window.dispatchEvent(new Event('finance-wasm-ready'));
}

//запуск загрузки wasm с перехватом ошибок
initFinanceWasm().catch(err => {
    console.error('Ошибка инициализации модуля WebAssembly:', err);
    if (typeof showToast === 'function') {
        showToast('Не удалось загрузить WebAssembly ядро. Обновите страницу.', 'error');
    }
});
