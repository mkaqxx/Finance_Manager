let allTransactions = []; // Сохраняем полученные с бэкенда транзакции

async function loadTransactions() {
    try {
        allTransactions = await windowAPI.getTransactions();

        // Заполняем фильтр счетов, если он есть на странице
        await populateAccountFilter();

        // Применяем текущие фильтры и отрисовываем
        applyFiltersAndRender();
    } catch (error) {
        console.error("Ошибка загрузки транзакций:", error);
    }
}

// Заполнение выпадающего списка счетов в панели фильтров
async function populateAccountFilter() {
    const accSelect = document.getElementById('tx-filter-account');
    if (!accSelect || accSelect.dataset.loaded) return; // Заполняем только один раз

    try {
        const accounts = await windowAPI.getAccounts();
        accSelect.innerHTML = '<option value="all">Все счета</option>';
        accounts.forEach(acc => {
            const opt = document.createElement('option');
            opt.value = String(acc.id);
            opt.textContent = acc.name;
            accSelect.appendChild(opt);
        });
        accSelect.dataset.loaded = 'true';
    } catch (e) {
        console.error("Ошибка загрузки счетов для фильтра:", e);
    }
}

// Фильтрация, сортировка и вызов рендера
function applyFiltersAndRender() {
    const searchInput = document.getElementById('tx-search');
    const typeSelect = document.getElementById('tx-filter-type');
    const accountSelect = document.getElementById('tx-filter-account');
    const sortSelect = document.getElementById('tx-sort');

    const searchQuery = searchInput ? searchInput.value.toLowerCase().trim() : '';
    const typeFilter = typeSelect ? typeSelect.value : 'all';
    const accountFilter = accountSelect ? accountSelect.value : 'all';
    const sortBy = sortSelect ? sortSelect.value : 'date-desc';

    // 1. Фильтрация
    let filtered = allTransactions.filter(tx => {
        // Фильтр по типу
        if (typeFilter !== 'all' && tx.type !== typeFilter) return false;

        // Фильтр по счёту
        if (accountFilter !== 'all') {
            const accId = Number(accountFilter);
            const matchesAccount = (tx.account_id === accId) || (tx.destination_id === accId);
            if (!matchesAccount) return false;
        }

        // Поиск по сумме, дате или названию категории
        if (searchQuery) {
            const category = categoriesMap[tx.category_id] || { name: "" };
            const matchesCategory = category.name.toLowerCase().includes(searchQuery);
            const matchesAmount = String(tx.amount).includes(searchQuery);
            const matchesDate = tx.date && tx.date.includes(searchQuery);
            if (!matchesCategory && !matchesAmount && !matchesDate) return false;
        }

        return true;
    });

    // 2. Сортировка
    filtered.sort((a, b) => {
        if (sortBy === 'date-desc') return b.date.localeCompare(a.date);
        if (sortBy === 'date-asc') return a.date.localeCompare(b.date);
        if (sortBy === 'amount-desc') return b.amount - a.amount;
        if (sortBy === 'amount-asc') return a.amount - b.amount;
        return 0;
    });

    // 3. Отрисовка
    renderTransactionsTable(filtered);
}

// Отрисовка строк по готовому шаблону
function renderTransactionsTable(transactions) {
    const tbody = document.getElementById('transactions-table');
    const template = document.getElementById('tpl-transaction-row');
    if (!tbody || !template) return;

    tbody.innerHTML = '';

    transactions.forEach(tx => {
        const clone = template.content.cloneNode(true);
        const category = categoriesMap[tx.category_id] || { name: "-", color: "#ffffff" };
        const sign = tx.type === "income" ? '+' : '-';

        clone.querySelector('.tx-date').textContent = tx.date;
        const catCell = clone.querySelector('.tx-category');
        catCell.textContent = category.name;
        catCell.style.color = category.color;
        clone.querySelector('.tx-amount').textContent = `${sign}${tx.amount.toLocaleString()} Br`;
        clone.querySelector('.tx-delete-btn').onclick = () => handleRemoveTransaction(tx);

        tbody.appendChild(clone);
    });
}

// Привязываем события фильтрации (поиск и выбор из селектов)
document.getElementById('tx-search')?.addEventListener('input', applyFiltersAndRender);
document.getElementById('tx-filter-type')?.addEventListener('change', applyFiltersAndRender);
document.getElementById('tx-filter-account')?.addEventListener('change', applyFiltersAndRender);
document.getElementById('tx-sort')?.addEventListener('change', applyFiltersAndRender);

async function handleRemoveTransaction(tx){
    const ok = await showConfirm({
        title: 'Удаление транзакции',
        message: 'Удалить эту операцию? Баланс счёта будет пересчитан.',
        confirmText: 'Удалить'
    });
    if (!ok) return;
    try {
        await windowAPI.removeTransaction(tx.id);
        loadTransactions();
    } catch (e) {
        console.error("Ошибка удаления: " + e.message);
    }
}