//кэш полученных транзакций для быстрой фильтрации в памяти
let allTransactions = []; 
//текущий выбранный фильтр периода
let currentPeriodFilter = 'all';

//отрисовка скелетона во время загрузки данных
function renderTransactionsSkeleton() {
    const tbody = document.getElementById('transactions-table');
    if (!tbody) return;

    let skeletonHtml = '';
    for (let i = 0; i < 4; i++) {
        skeletonHtml += `
            <tr class="skeleton-row" style="border-bottom: 1px solid var(--border-color);">
                <td style="padding: 14px 16px;"><div class="skeleton skeleton-text" style="width: 80px; height: 14px; margin: 0;"></div></td>
                <td style="padding: 14px 16px;"><div class="skeleton skeleton-text" style="width: 140px; height: 14px; margin: 0;"></div></td>
                <td style="padding: 14px 16px;"><div class="skeleton skeleton-text" style="width: 90px; height: 16px; margin: 0;"></div></td>
                <td style="padding: 14px 16px; width: 44px;"><div class="skeleton skeleton-text" style="width: 20px; height: 14px; margin: 0;"></div></td>
            </tr>
        `;
    }
    tbody.innerHTML = skeletonHtml;
}

//загрузка списка транзакций из с++
async function loadTransactions() {
    try {
        renderTransactionsSkeleton();
        allTransactions = await windowAPI.getTransactions();
        await populateAccountFilter();
        applyFiltersAndRender();
    } catch (error) {
        console.error("Ошибка загрузки транзакций:", error);
    }
}

//заполнение выпадающего списка счетов в панели фильтра
async function populateAccountFilter() {
    const accSelect = document.getElementById('tx-filter-account');
    if (!accSelect || accSelect.dataset.loaded) return;

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

//фильтрация, сортировка и вызов рендера таблицы
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

    let filtered = allTransactions.filter(tx => {
        if (typeFilter !== 'all' && tx.type !== typeFilter) return false;

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

        if (accountFilter !== 'all') {
            const accId = Number(accountFilter);
            const matchesAccount = (tx.account_id === accId) || (tx.destination_id === accId);
            if (!matchesAccount) return false;
        }

        if (searchQuery) {
            const category = categoriesMap[tx.category_id] || { name: "" };
            const matchesCategory = category.name.toLowerCase().includes(searchQuery);
            const matchesAmount = String(tx.amount).includes(searchQuery);
            const matchesDate = tx.date && tx.date.includes(searchQuery);
            if (!matchesCategory && !matchesAmount && !matchesDate) return false;
        }

        return true;
    });

    filtered.sort((a, b) => {
        if (sortBy === 'date-desc') return (b.date || '').localeCompare(a.date || '');
        if (sortBy === 'date-asc') return (a.date || '').localeCompare(b.date || '');
        if (sortBy === 'amount-desc') return (b.amount || 0) - (a.amount || 0);
        if (sortBy === 'amount-asc') return (a.amount || 0) - (b.amount || 0);
        return 0;
    });

    renderTransactionsTable(filtered);
}

//отрисовка строк таблицы по html-шаблону
function renderTransactionsTable(transactions) {
    const tbody = document.getElementById('transactions-table');
    const template = document.getElementById('tpl-transaction-row');
    if (!tbody || !template) return;

    tbody.innerHTML = '';

    if (!transactions.length) {
        tbody.innerHTML = `
            <tr class="empty-row">
                <td colspan="4" style="padding: 36px 0; border: none; background: transparent !important;">
                    <div class="empty-state">
                        <div class="empty-state-icon">
                            <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"/><path d="m21 21-4.3-4.3"/></svg>
                        </div>
                        <h4>Операций не найдено</h4>
                        <p>По выбранным параметрам нет подходящих записей или список пуст.</p>
                        <button type="button" onclick="openTxModal()" class="btn-primary" style="font-size: 13px;">+ Создать операцию <span class="kbd-hint">N</span></button>
                    </div>
                </td>
            </tr>
        `;
        return;
    }

    transactions.forEach((tx) => {
        const clone = template.content.cloneNode(true);
        const category = categoriesMap[tx.category_id] || { name: (tx.type === 'transfer' ? 'Перевод' : 'Без категории'), color: "var(--text-title)" };
        const isInc = tx.type === "income";
        const isTransfer = tx.type === "transfer";
        const sign = isInc ? '+' : isTransfer ? '' : '−';

        const badge = clone.querySelector('.tx-badge');
        if (badge) {
            if (isInc) {
                badge.className = 'tx-badge income';
                badge.textContent = '↑';
            } else if (isTransfer) {
                badge.className = 'tx-badge transfer';
                badge.textContent = '⇄';
            } else {
                badge.className = 'tx-badge expense';
                badge.textContent = '↓';
            }
        }

        let dateFormatted = tx.date || '';
        if (tx.date && tx.date.length === 10) {
            const parts = tx.date.split('-');
            if (parts.length === 3) dateFormatted = `${parts[2]}.${parts[1]}.${parts[0]}`;
        }
        clone.querySelector('.tx-date').textContent = dateFormatted;

        const catCell = clone.querySelector('.tx-category');
        catCell.textContent = category.name;
        catCell.style.color = category.color || 'var(--text-title)';

        const cur = (tx.currency || 'byn').toLowerCase();
        const sym = CURRENCY_SYMBOLS[cur] || cur.toUpperCase();
        const amountEl = clone.querySelector('.tx-amount');
        amountEl.textContent = `${sign}${Number(tx.amount).toLocaleString()} ${sym}`;

        if (isInc) {
            amountEl.style.color = 'var(--color-income)';
        } else if (isTransfer) {
            amountEl.style.color = 'var(--color-transfer)';
        } else {
            amountEl.style.color = 'var(--text-title)';
        }

        clone.querySelector('.tx-delete-btn').onclick = (e) => {
            const tr = e.target.closest('tr');
            handleRemoveTransaction(tx, tr);
        };

        tbody.appendChild(clone);
    });
}

//привязка событий кнопок быстрых фильтров (чипсов)
function initTransactionChips() {
    const typeButtons = document.querySelectorAll('#tx-type-chips .filter-chip');
    typeButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            typeButtons.forEach(c => c.classList.remove('active'));
            btn.classList.add('active');
            const typeSelect = document.getElementById('tx-filter-type');
            if (typeSelect) typeSelect.value = btn.dataset.type;
            applyFiltersAndRender();
        });
    });

    const periodButtons = document.querySelectorAll('#tx-period-chips .filter-chip');
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

document.getElementById('tx-search')?.addEventListener('input', applyFiltersAndRender);
document.getElementById('tx-filter-type')?.addEventListener('change', applyFiltersAndRender);
document.getElementById('tx-filter-account')?.addEventListener('change', applyFiltersAndRender);
document.getElementById('tx-sort')?.addEventListener('change', applyFiltersAndRender);

//удаление транзакции с подтверждением
async function handleRemoveTransaction(tx, row) {
    const ok = await showConfirm({
        title: 'Удаление операции',
        message: 'Удалить эту операцию? Баланс привязанного счёта будет пересчитан.',
        confirmText: 'Удалить'
    });
    if (!ok) return;

    try {
        if (row) {
            row.style.opacity = '0';
            row.style.transform = 'translateX(20px)';
            await new Promise(r => setTimeout(r, 150));
        }

        await windowAPI.removeTransaction(tx.id);
        if (typeof showToast === 'function') showToast('Операция удалена', 'info');

        loadTransactions();
        if (document.getElementById('view-dashboard').style.display === 'block') loadDashboard();
    } catch (e) {
        if (row) {
            row.style.opacity = '1';
            row.style.transform = 'none';
        }
        console.error("Ошибка удаления транзакции:", e);
        if (typeof showToast === 'function') showToast(e.message, 'error');
    }
}