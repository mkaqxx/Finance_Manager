let allTransactions = []; // Сохраняем полученные с бэкенда транзакции
let currentPeriodFilter = 'all';

function renderTransactionsSkeleton() {
    const tbody = document.getElementById('transactions-table');
    if (!tbody) return;
    let skeletonHtml = '';
    for (let i = 0; i < 4; i++) {
        skeletonHtml += `
            <tr class="skeleton-row" style="border-bottom: 1px solid var(--border-color);">
                <td style="padding: 16px 8px;"><div class="skeleton skeleton-text" style="width: 80px; height: 14px; margin: 0;"></div></td>
                <td style="padding: 16px 8px;"><div class="skeleton skeleton-text" style="width: 140px; height: 14px; margin: 0;"></div></td>
                <td style="padding: 16px 8px;"><div class="skeleton skeleton-text" style="width: 90px; height: 16px; margin: 0;"></div></td>
                <td style="padding: 16px 8px; width: 40px;"><div class="skeleton skeleton-text" style="width: 20px; height: 14px; margin: 0;"></div></td>
            </tr>
        `;
    }
    tbody.innerHTML = skeletonHtml;
}

async function loadTransactions() {
    try {
        renderTransactionsSkeleton();
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

    const now = new Date();
    const currentYear = now.getFullYear();
    const currentMonth = now.getMonth();

    // 1. Фильтрация
    let filtered = allTransactions.filter(tx => {
        // Фильтр по типу
        if (typeFilter !== 'all' && tx.type !== typeFilter) return false;

        // Фильтр по периоду
        if (currentPeriodFilter !== 'all' && tx.date) {
            const txDate = new Date(tx.date);
            if (currentPeriodFilter === 'this_month') {
                if (txDate.getFullYear() !== currentYear || txDate.getMonth() !== currentMonth) return false;
            } else if (currentPeriodFilter === 'prev_month') {
                const prevMonthDate = new Date(currentYear, currentMonth - 1, 1);
                if (txDate.getFullYear() !== prevMonthDate.getFullYear() || txDate.getMonth() !== prevMonthDate.getMonth()) return false;
            } else if (currentPeriodFilter === 'last_30') {
                const thirtyDaysAgo = new Date();
                thirtyDaysAgo.setDate(thirtyDaysAgo.getDate() - 30);
                if (txDate < thirtyDaysAgo) return false;
            }
        }

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

    if (!transactions.length) {
        tbody.innerHTML = `
            <tr class="empty-row">
                <td colspan="4" style="padding: 32px 0; border: none; background: transparent !important;">
                    <div class="empty-state">
                        <div class="empty-state-icon">
                            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
                        </div>
                        <h4>Операций не найдено</h4>
                        <p>По выбранным фильтрам нет подходящих транзакций или список пуст.</p>
                        <button onclick="openTxModal()" class="btn-primary" style="font-size: 13px; pointer-events: auto;">+ Создать операцию <span class="kbd-hint">[N]</span></button>
                    </div>
                </td>
            </tr>
        `;
        return;
    }

    transactions.forEach((tx, idx) => {
        const clone = template.content.cloneNode(true);
        const row = clone.querySelector('tr');
        if (row) {
            row.classList.add('animate-cascade');
            row.style.setProperty('--item-idx', Math.min(idx, 25));
        }
        const category = categoriesMap[tx.category_id] || { name: "-", color: "var(--text-title)" };
        const sign = tx.type === "income" ? '+' : '-';

        const badge = clone.querySelector('.tx-badge');
        if (badge) {
            if (tx.type === 'income') {
                badge.className = 'tx-badge income';
                badge.textContent = '↑';
            } else if (tx.type === 'expense' || tx.type === 'regular_expense') {
                badge.className = 'tx-badge expense';
                badge.textContent = '↓';
            } else {
                badge.className = 'tx-badge transfer';
                badge.textContent = '⇄';
            }
        }

        clone.querySelector('.tx-date').textContent = tx.date;
        const catCell = clone.querySelector('.tx-category');
        catCell.textContent = category.name;
        catCell.style.color = category.color || 'var(--text-title)';
        const cur = tx.currency;
        const sym = CURRENCY_SYMBOLS[cur] || cur;
        const amountEl = clone.querySelector('.tx-amount');
        amountEl.textContent = `${sign}${tx.amount.toLocaleString()} ${sym}`;
        amountEl.style.color = tx.type === 'income' ? 'var(--color-income)' : 'var(--text-title)';
        clone.querySelector('.tx-delete-btn').onclick = (e) => {
            const tr = e.target.closest('tr');
            handleRemoveTransaction(tx, tr);
        };

        tbody.appendChild(clone);
    });
}

// Привязываем события быстрых чипсов и сегментированных кнопок
function initTransactionChips() {
    const typeButtons = document.querySelectorAll('#tx-type-chips .segmented-btn, #tx-type-chips .filter-chip');
    typeButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            typeButtons.forEach(c => c.classList.remove('active'));
            btn.classList.add('active');
            const typeSelect = document.getElementById('tx-filter-type');
            if (typeSelect) typeSelect.value = btn.dataset.type;
            applyFiltersAndRender();
        });
    });

    const periodButtons = document.querySelectorAll('#tx-period-chips .segmented-btn, #tx-period-chips .filter-chip');
    periodButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            periodButtons.forEach(c => c.classList.remove('active'));
            btn.classList.add('active');
            currentPeriodFilter = btn.dataset.period;
            applyFiltersAndRender();
        });
    });
}

initTransactionChips();

// Привязываем события фильтрации (поиск и выбор из селектов)
document.getElementById('tx-search')?.addEventListener('input', applyFiltersAndRender);
document.getElementById('tx-filter-type')?.addEventListener('change', applyFiltersAndRender);
document.getElementById('tx-filter-account')?.addEventListener('change', applyFiltersAndRender);
document.getElementById('tx-sort')?.addEventListener('change', applyFiltersAndRender);

async function handleRemoveTransaction(tx, row) {
    const ok = await showConfirm({
        title: 'Удаление транзакции',
        message: 'Удалить эту операцию? Баланс счёта будет пересчитан.',
        confirmText: 'Удалить'
    });
    if (!ok) return;
    try {
        if (row) {
            row.classList.add('row-deleting');
            await new Promise(r => setTimeout(r, 260));
        }
        await windowAPI.removeTransaction(tx.id);
        if (typeof showToast === 'function') showToast('Операция удалена', 'info');
        loadTransactions();
    } catch (e) {
        if (row) row.classList.remove('row-deleting');
        console.error("Ошибка удаления: " + e.message);
        if (typeof showToast === 'function') showToast(e.message, 'error');
    }
}