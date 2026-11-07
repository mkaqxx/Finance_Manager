//переключение видимости полей в зависимости от типа операции
function toggleTxFields() {
    const type = document.getElementById('tx-type').value;

    //категория нужна для доходов и расходов, но скрыта для переводов
    document.getElementById('tx-category').style.display = type === 'transfer' ? 'none' : 'block';
    document.getElementById('tx-category').required = type !== 'transfer';

    //счет зачисления нужен только при переводе между своими счетами
    document.getElementById('tx-dest-account').style.display = type === 'transfer' ? 'block' : 'none';
    document.getElementById('tx-dest-account').required = type === 'transfer';

    //периодичность нужна только для регулярных расходов
    document.getElementById('tx-period').style.display = type === 'regular_expense' ? 'block' : 'none';
    document.getElementById('tx-period').required = type === 'regular_expense';

    //сброс списка счетов зачисления если не перевод
    if (type !== 'transfer') {
        const destSelect = document.getElementById('tx-dest-account');
        destSelect.innerHTML = '<option value="" disabled selected>Счет зачисления...</option>';
    }
}

//открытие модального окна добавления транзакции с опциональным пресетом типа
async function openTxModal(defaultType = null) {
    document.getElementById('tx-modal').style.display = 'flex';
    document.getElementById('tx-form').reset();
    document.getElementById('tx-date').valueAsDate = new Date();

    if (defaultType) {
        const typeSelect = document.getElementById('tx-type');
        if (typeSelect) typeSelect.value = defaultType;
    }

    toggleTxFields();

    //очистка списков счетов
    const accSelect = document.getElementById('tx-account');
    const destSelect = document.getElementById('tx-dest-account');
    accSelect.innerHTML = '<option value="" disabled selected>Счет списания...</option>';
    destSelect.innerHTML = '<option value="" disabled selected>Счет зачисления...</option>';

    try {
        const accounts = await windowAPI.getAccounts();
        accounts.forEach(acc => {
            const sym = CURRENCY_SYMBOLS[(acc.currency || 'byn').toLowerCase()] || acc.currency;
            const opt = `<option value="${acc.id}">${acc.name} (${acc.balance.toLocaleString()} ${sym})</option>`;
            accSelect.innerHTML += opt;
            destSelect.innerHTML += opt;
        });

        //при переводе фильтруем счета зачисления в той же валюте
        accSelect.onchange = () => {
            const type = document.getElementById('tx-type').value;
            if (type !== 'transfer') return;

            const srcId = parseInt(accSelect.value);
            const src = accounts.find(a => a.id === srcId);
            if (!src) return;

            destSelect.innerHTML = '<option value="" disabled selected>Счет зачисления...</option>';
            accounts
                .filter(a => a.currency === src.currency && a.id !== srcId)
                .forEach(acc => {
                    const sym = CURRENCY_SYMBOLS[(acc.currency || 'byn').toLowerCase()] || acc.currency;
                    destSelect.innerHTML += `<option value="${acc.id}">${acc.name} (${acc.balance.toLocaleString()} ${sym})</option>`;
                });
        };
    } catch (e) {
        console.error("Ошибка загрузки счетов в модалку:", e);
    }

    //заполнение категорий из словаря
    const catSelect = document.getElementById('tx-category');
    catSelect.innerHTML = '<option value="" disabled selected>Выберите категорию...</option>';
    for (const [id, cat] of Object.entries(categoriesMap)) {
        catSelect.innerHTML += `<option value="${id}">${cat.name}</option>`;
    }

    //фокус в поле суммы
    setTimeout(() => {
        document.getElementById('tx-amount')?.focus();
    }, 50);
}

//закрытие окна транзакций
function closeTxModal() { 
    closeModalAnimated('tx-modal'); 
}

//обработка отправки формы транзакции
document.getElementById('tx-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const type = document.getElementById('tx-type').value;

    const data = {
        amount: parseFloat(document.getElementById('tx-amount').value),
        date: document.getElementById('tx-date').value,
        type: type,
        account_id: parseInt(document.getElementById('tx-account').value),
        category_id: type === 'transfer' ? 0 : parseInt(document.getElementById('tx-category').value)
    };

    if (type === 'transfer') {
        data.destination_id = parseInt(document.getElementById('tx-dest-account').value);
    } else if (type === 'regular_expense') {
        data.period = document.getElementById('tx-period').value;
    }

    try {
        await windowAPI.addTransaction(data);
        closeTxModal();

        if (typeof showToast === 'function') showToast('Операция успешно добавлена', 'success');

        if (type === 'income' && typeof triggerCelebrationConfetti === 'function') {
            triggerCelebrationConfetti();
        }

        if (document.getElementById('view-transactions').style.display === 'block') loadTransactions();
        if (document.getElementById('view-dashboard').style.display === 'block') loadDashboard();
        if (document.getElementById('view-accounts').style.display === 'block') loadAccounts();
    } catch (error) {
        if (typeof showToast === 'function') showToast(error.message, 'error');
        else alert(error.message);
    }
});

//хранит id редактируемого счета
let editingAccountId = null;

//переключение полей типа счета
function toggleAccountFields() {
    const type = document.getElementById('acc-type').value;

    document.getElementById('acc-bank-digits').style.display = type === 'bank_account' ? 'block' : 'none';
    document.getElementById('acc-bank-digits').required = type === 'bank_account';

    document.getElementById('acc-goal').style.display = type === 'savings_account' ? 'block' : 'none';
    document.getElementById('acc-goal').required = type === 'savings_account';
    document.getElementById('acc-deadline').style.display = type === 'savings_account' ? 'block' : 'none';
    document.getElementById('acc-deadline').required = type === 'savings_account';
}

//открытие модального окна добавления или редактирования счета
function openAccountModal(editData = null) {
    const modal = document.getElementById('account-modal');
    const form = document.getElementById('account-form');
    const typeSelect = document.getElementById('acc-type');
    const currSelect = document.getElementById('acc-currency');
    const title = modal.querySelector('h2');
    const submitBtn = modal.querySelector('button[type="submit"]');

    modal.style.display = 'flex';
    form.reset();

    if (editData) {
        editingAccountId = editData.id;
        title.textContent = 'Редактировать счет';
        submitBtn.textContent = 'Сохранить';

        document.getElementById('acc-name').value = editData.name;
        document.getElementById('acc-balance').value = editData.balance;
        currSelect.value = (editData.currency || 'byn').toLowerCase();
        currSelect.disabled = true;

        typeSelect.value = editData.type;
        typeSelect.disabled = true;

        if (editData.type === 'bank_account') {
            document.getElementById('acc-bank-digits').value = editData.number || editData.last_four_digits || '';
        } else if (editData.type === 'savings_account') {
            document.getElementById('acc-goal').value = editData.goal || editData.goal_amount || '';
            document.getElementById('acc-deadline').value = editData.deadline || '';
        }
    } else {
        editingAccountId = null;
        title.textContent = 'Новый счет';
        submitBtn.textContent = 'Сохранить';
        currSelect.disabled = false;
        typeSelect.disabled = false;
        document.getElementById('acc-deadline').valueAsDate = new Date();
    }

    toggleAccountFields();
    setTimeout(() => {
        document.getElementById('acc-name')?.focus();
    }, 50);
}

//закрытие формы счета
function closeAccountModal() { 
    closeModalAnimated('account-modal'); 
}

// ========================================================
// ВЫДВИЖНАЯ БОКОВАЯ ПАНЕЛЬ СПРАВА (SHEET / DRAWER 400PX)
// ДЛЯ ДЕТАЛЬНОГО ПРОСМОТРА СЧЕТА (#account-detail-modal)
// ========================================================

function openAccountDetailModal(acc) {
    const sheet = document.getElementById('account-detail-modal');
    if (!sheet) return;

    // Сброс классов и открытие панели
    sheet.classList.remove('sheet-closing');
    sheet.classList.add('sheet-open');

    // Название счета
    document.getElementById('modal-acc-name').textContent = acc.name;

    const cur = (acc.currency || 'byn').toLowerCase();
    const sym = CURRENCY_SYMBOLS[cur] || acc.currency.toUpperCase();

    let typeLabel = 'Наличные';
    if (acc.type === 'bank_account') typeLabel = 'Банковский счёт / Карта';
    else if (acc.type === 'savings_account') typeLabel = 'Копилка / Финансовая цель';

    let infoHtml = `
        <div style="background: var(--bg-hover); border: 1px solid var(--border-color); border-radius: var(--card-radius); padding: 16px; margin-bottom: 16px;">
            <div style="display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 8px;">
                <span style="font-size: 12px; font-weight: 600; color: var(--text-muted); text-transform: uppercase;">Текущий баланс</span>
                <span class="badge-pill">${acc.currency.toUpperCase()}</span>
            </div>
            <div class="amount" style="font-size: 26px; font-weight: 700; color: var(--text-title); line-height: 1.2;">
                ${Number(acc.balance).toLocaleString()} <span style="font-size: 16px; color: var(--text-muted);">${sym}</span>
            </div>
            <div style="font-size: 12.5px; color: var(--text-muted); margin-top: 6px;">
                Тип: <strong style="color: var(--text-title);">${typeLabel}</strong>
            </div>
        </div>
    `;

    // Инфо о цели для накопительного счета
    if (acc.type === 'savings_account') {
        const isReached = (acc.progress || 0) >= 100;
        const goalVal = acc.goal || acc.goal_amount || 0;
        const progressVal = Math.min(Math.max(acc.progress || 0, 0), 100);

        infoHtml += `
            <div style="background: var(--bg-hover); border: 1px solid var(--border-color); border-radius: var(--card-radius); padding: 16px; margin-bottom: 16px;">
                <div style="display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 6px;">
                    <span style="font-size: 12px; font-weight: 600; color: var(--text-muted); text-transform: uppercase;">Прогресс накопления</span>
                    <span class="tab-num" style="font-weight: 700; color: var(--text-title);">${progressVal.toFixed(1)}%</span>
                </div>
                <div style="width: 100%; height: 8px; background: var(--bg-card); border: 1px solid var(--border-color); border-radius: 4px; overflow: hidden; margin-bottom: 10px;">
                    <div style="width: ${progressVal}%; height: 100%; background: var(--accent-color); border-radius: 3px; transition: width 0.5s ease;"></div>
                </div>
                <div style="display: flex; flex-direction: column; gap: 4px; font-size: 12.5px; color: var(--text-muted);">
                    <div>Цель: <strong class="tab-num" style="color: var(--text-title);">${Number(goalVal).toLocaleString()} ${sym}</strong></div>
                    <div>Нужно откладывать: <strong class="tab-num" style="color: var(--text-title);">${Number(acc.monthly_required || 0).toLocaleString()} ${sym}/мес</strong></div>
                    <div>Крайний срок: <strong class="tab-num" style="color: var(--text-title);">${acc.deadline || '—'}</strong></div>
                </div>
                ${isReached ? '<div style="margin-top: 10px; padding: 6px 10px; background: var(--color-income-subtle); color: var(--color-income); font-size: 12px; font-weight: 600; border-radius: 4px;">🎉 Финансовая цель успешно достигнута!</div>' : ''}
            </div>
        `;
    }

    if (acc.type === 'bank_account' && (acc.number || acc.last_four_digits)) {
        infoHtml += `
            <div style="background: var(--bg-hover); border: 1px solid var(--border-color); border-radius: var(--card-radius); padding: 12px 16px; margin-bottom: 16px; font-size: 13px;">
                Номер карты: <strong class="tab-num" style="color: var(--text-title);">•••• ${acc.number || acc.last_four_digits}</strong>
            </div>
        `;
    }

    document.getElementById('modal-acc-info').innerHTML = infoHtml;

    // Загрузка последних транзакций по счету
    loadAccountTransactions(acc.id);

    // Кнопка удаления
    const deleteBtn = document.getElementById('btn-delete-account');
    if (deleteBtn) {
        deleteBtn.onclick = async () => {
            const ok = await showConfirm({
                title: 'Удаление счёта',
                message: `Удалить счёт «${acc.name}»? Все привязанные операции будут также удалены.`,
                confirmText: 'Удалить'
            });
            if (!ok) return;

            try {
                await windowAPI.removeAccount(acc.id);
                if (typeof showToast === 'function') showToast(`Счёт «${acc.name}» удалён`, 'info');
                closeAccountDetailModal();
                if (document.getElementById('view-accounts').style.display === 'block') loadAccounts();
                if (document.getElementById('view-dashboard').style.display === 'block') loadDashboard();
            } catch (e) {
                console.error("Ошибка удаления счета:", e);
                if (typeof showToast === 'function') showToast(e.message, 'error');
            }
        };
    }

    // Кнопка редактирования
    const editBtn = document.getElementById('btn-edit-account');
    if (editBtn) {
        editBtn.onclick = () => {
            closeAccountDetailModal();
            openAccountModal(acc);
        };
    }
}

// Закрытие Drawer панели справа
function closeAccountDetailModal() {
    const sheet = document.getElementById('account-detail-modal');
    if (!sheet || !sheet.classList.contains('sheet-open')) return;

    sheet.classList.add('sheet-closing');
    setTimeout(() => {
        sheet.classList.remove('sheet-open');
        sheet.classList.remove('sheet-closing');
    }, 220);
}

// Загрузка истории транзакций для Sheet
async function loadAccountTransactions(accountId) {
    const container = document.getElementById('modal-acc-transactions');
    container.innerHTML = '<span style="color: var(--text-muted); font-size: 13px;">Загрузка операций...</span>';

    try {
        const transactions = await windowAPI.getTransactionsByAccount(accountId);
        container.innerHTML = '';

        if (!transactions || !transactions.length) {
            container.innerHTML = '<span style="color: var(--text-muted); font-size: 13px;">Операций по этому счету пока нет.</span>';
            return;
        }

        transactions.slice(0, 10).forEach(tx => {
            const cat = categoriesMap[tx.category_id] || { name: 'Операция', color: 'var(--text-title)' };
            const isInc = tx.type === 'income';
            const sign = isInc ? '+' : '-';
            const cur = (tx.currency || 'byn').toLowerCase();
            const sym = CURRENCY_SYMBOLS[cur] || cur.toUpperCase();

            const row = document.createElement('div');
            row.style.cssText = 'display:flex; justify-content:space-between; align-items:center; padding:10px 12px; background:var(--bg-hover); border:1px solid var(--border-color); border-radius:var(--btn-radius); font-size:13px;';

            row.innerHTML = `
                <div style="display: flex; flex-direction: column; gap: 2px;">
                    <span style="font-weight: 600; color: var(--text-title);">${cat.name}</span>
                    <span class="tx-date" style="color: var(--text-muted); font-size: 11.5px;">${tx.date}</span>
                </div>
                <span class="amount" style="font-weight: 700; color: ${isInc ? 'var(--color-income)' : 'var(--text-title)'};">
                    ${sign}${tx.amount.toLocaleString()} ${sym}
                </span>
            `;
            container.appendChild(row);
        });
    } catch (e) {
        console.error("Ошибка загрузки операций счета:", e);
        container.innerHTML = '<span style="color: var(--color-expense); font-size: 13px;">Ошибка загрузки</span>';
    }
}

// Сохранение счета
document.getElementById('account-form').addEventListener('submit', async (e) => {
    e.preventDefault();

    const type = document.getElementById('acc-type').value;
    const data = {
        name: document.getElementById('acc-name').value,
        balance: parseFloat(document.getElementById('acc-balance').value),
        currency: document.getElementById('acc-currency').value,
        type: type
    };

    if (type === 'bank_account') {
        data.last_four_digits = document.getElementById('acc-bank-digits').value;
    } else if (type === 'savings_account') {
        data.goal_amount = parseFloat(document.getElementById('acc-goal').value);
        data.deadline = document.getElementById('acc-deadline').value;
    }

    try {
        if (editingAccountId) {
            data.id = editingAccountId;
            await windowAPI.editAccount(data);
            if (typeof showToast === 'function') showToast('Счёт успешно обновлён', 'success');
        } else {
            await windowAPI.addAccount(data);
            if (typeof showToast === 'function') showToast('Счёт успешно добавлен', 'success');
        }

        closeAccountModal();
        if (document.getElementById('view-accounts').style.display === 'block') loadAccounts();
        if (document.getElementById('view-dashboard').style.display === 'block') loadDashboard();
        closeAccountDetailModal();
    } catch (error) {
        console.error("Ошибка сохранения счета:", error);
        if (typeof showToast === 'function') showToast(error.message, 'error');
    }
});

// Хранит id редактируемого бюджета
let editingBudgetCategId = null;

// Модальное окно бюджета
async function openBudgetModal(editData = null) {
    const modal = document.getElementById('budget-modal');
    const form = document.getElementById('budget-form');
    const title = modal.querySelector('h2');
    const submitBtn = modal.querySelector('button[type="submit"]');
    const catSelect = document.getElementById('budget-category');

    form.reset();

    if (editData) {
        editingBudgetCategId = editData.category_id;
        title.textContent = 'Редактировать лимит';
        submitBtn.textContent = 'Сохранить';

        catSelect.innerHTML = `<option value="${editData.category_id}" selected>${editData.category_name || editData.name}</option>`;
        catSelect.disabled = true;
        document.getElementById('budget-limit').value = editData.limit;
    } else {
        editingBudgetCategId = null;
        title.textContent = 'Установить лимит расходов';
        submitBtn.textContent = 'Сохранить';
        catSelect.disabled = false;
        catSelect.innerHTML = '<option value="" disabled selected>Загрузка категорий...</option>';

        try {
            const categories = await windowAPI.getCategories();
            catSelect.innerHTML = '<option value="" disabled selected>Выберите категорию...</option>';
            const expenseCategories = categories.filter(c => (c.type || '').toLowerCase() === 'expense');
            const listToRender = expenseCategories.length > 0 ? expenseCategories : categories;

            listToRender.forEach(cat => {
                const opt = document.createElement('option');
                opt.value = cat.id;
                opt.textContent = cat.name;
                catSelect.appendChild(opt);
            });
        } catch (err) {
            console.error("Ошибка категорий бюджета:", err);
            catSelect.innerHTML = '<option value="" disabled selected>Ошибка загрузки</option>';
        }
    }

    modal.style.display = 'flex';
    setTimeout(() => {
        document.getElementById('budget-limit')?.focus();
    }, 50);
}

document.getElementById('budget-form').onsubmit = async (e) => {
    e.preventDefault();

    const limitValue = parseFloat(document.getElementById('budget-limit').value);
    const catId = editingBudgetCategId !== null
        ? editingBudgetCategId
        : parseInt(document.getElementById('budget-category').value);

    const data = {
        category_id: Number(catId),
        limit: limitValue
    };

    try {
        if (editingBudgetCategId) {
            await windowAPI.editBudget(data);
            if (typeof showToast === 'function') showToast('Лимит бюджета обновлён', 'success');
        } else {
            await windowAPI.addBudget(data);
            if (typeof showToast === 'function') showToast('Лимит бюджета установлен', 'success');
        }

        closeBudgetModal();
        if (document.getElementById('view-budget').style.display === 'block') renderBudgets();
        if (document.getElementById('view-dashboard').style.display === 'block') loadDashboard();
    } catch (error) {
        if (typeof showToast === 'function') showToast(error.message, 'error');
        else alert("Ошибка: " + error.message);
    }
};

function closeBudgetModal() { 
    closeModalAnimated('budget-modal'); 
}

// Модальное окно категорий
function openCategoryModal() { 
    document.getElementById('category-modal').style.display = 'flex'; 
    document.getElementById('category-form').reset(); 
    setTimeout(() => {
        document.getElementById('cat-name')?.focus();
    }, 50);
}

function closeCategoryModal() { 
    closeModalAnimated('category-modal'); 
}

document.getElementById('category-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    try {
        await windowAPI.addCategory({
            name: document.getElementById('cat-name').value,
            type: document.getElementById('cat-type').value,
            color: document.getElementById('cat-color').value
        });

        closeCategoryModal();
        await loadCategories();

        if (typeof showToast === 'function') showToast('Категория успешно создана', 'success');
        if (document.getElementById('view-budget').style.display === 'block') renderCategories();
    } catch (error) {
        if (typeof showToast === 'function') showToast(error.message, 'error');
        else alert(error.message);
    }
});

// Диалог подтверждения
function showConfirm({ title = 'Подтверждение', message = 'Вы уверены?', confirmText = 'Удалить', isDanger = true }) {
    return new Promise((resolve) => {
        const modal = document.getElementById('confirm-modal');
        const titleEl = document.getElementById('confirm-title');
        const msgEl = document.getElementById('confirm-message');
        const okBtn = document.getElementById('confirm-btn-ok');
        const cancelBtn = document.getElementById('confirm-btn-cancel');

        titleEl.textContent = title;
        msgEl.textContent = message;
        okBtn.textContent = confirmText;
        okBtn.className = isDanger ? 'btn-danger' : 'btn-primary';

        modal.style.display = 'flex';

        function cleanup(result) {
            closeModalAnimated(modal);
            okBtn.onclick = null;
            cancelBtn.onclick = null;
            resolve(result);
        }

        okBtn.onclick = () => cleanup(true);
        cancelBtn.onclick = () => cleanup(false);
    });
}

// ========================================================
// COMMAND PALETTE (CTRL+K) РЕАЛИЗАЦИЯ
// ========================================================

const COMMAND_PALETTE_ACTIONS = [
    {
        id: 'new-expense',
        title: 'Добавить расход',
        category: 'Операции',
        icon: '↓',
        hint: 'N',
        handler: () => openTxModal('expense')
    },
    {
        id: 'new-income',
        title: 'Добавить доход',
        category: 'Операции',
        icon: '↑',
        hint: '',
        handler: () => openTxModal('income')
    },
    {
        id: 'new-transfer',
        title: 'Добавить перевод между счетами',
        category: 'Операции',
        icon: '⇄',
        hint: '',
        handler: () => openTxModal('transfer')
    },
    {
        id: 'new-account',
        title: 'Создать новый счет',
        category: 'Счета',
        icon: '💳',
        hint: '',
        handler: () => openAccountModal()
    },
    {
        id: 'new-budget',
        title: 'Установить лимит бюджета',
        category: 'Бюджет',
        icon: '🎯',
        hint: '',
        handler: () => openBudgetModal()
    },
    {
        id: 'new-category',
        title: 'Создать категорию',
        category: 'Бюджет',
        icon: '🏷️',
        hint: '',
        handler: () => openCategoryModal()
    },
    {
        id: 'nav-dashboard',
        title: 'Перейти на Главную',
        category: 'Навигация',
        icon: '📊',
        hint: '1',
        handler: () => switchPage('dashboard')
    },
    {
        id: 'nav-accounts',
        title: 'Перейти в Счета',
        category: 'Навигация',
        icon: '💳',
        hint: '2',
        handler: () => switchPage('accounts')
    },
    {
        id: 'nav-transactions',
        title: 'Перейти в Транзакции',
        category: 'Навигация',
        icon: '📜',
        hint: '3',
        handler: () => switchPage('transactions')
    },
    {
        id: 'nav-budget',
        title: 'Перейти в Бюджет',
        category: 'Навигация',
        icon: '🎯',
        hint: '4',
        handler: () => switchPage('budget')
    },
    {
        id: 'nav-analytics',
        title: 'Перейти в Аналитику',
        category: 'Навигация',
        icon: '📈',
        hint: '5',
        handler: () => switchPage('analytics')
    },
    {
        id: 'toggle-theme',
        title: 'Переключить тему оформления (Светлая / Темная)',
        category: 'Интерфейс',
        icon: '🌓',
        hint: '',
        handler: () => toggleTheme()
    }
];

let selectedCommandIndex = 0;
let filteredCommands = [];

function openCommandPalette() {
    const modal = document.getElementById('command-palette-modal');
    const input = document.getElementById('cmd-palette-input');
    if (!modal || !input) return;

    modal.style.display = 'flex';
    input.value = '';
    renderCommandPalette('');
    setTimeout(() => input.focus(), 40);
}

function closeCommandPalette() {
    closeModalAnimated('command-palette-modal');
}

function renderCommandPalette(query = '') {
    const list = document.getElementById('cmd-palette-list');
    if (!list) return;

    const q = query.toLowerCase().trim();
    filteredCommands = COMMAND_PALETTE_ACTIONS.filter(item => {
        return item.title.toLowerCase().includes(q) ||
               item.category.toLowerCase().includes(q) ||
               (item.hint && item.hint.toLowerCase().includes(q));
    });

    selectedCommandIndex = 0;
    list.innerHTML = '';

    if (!filteredCommands.length) {
        list.innerHTML = `
            <div style="padding: 24px; text-align: center; color: var(--text-muted); font-size: 13px;">
                Ничего не найдено по запросу «${query}»
            </div>
        `;
        return;
    }

    filteredCommands.forEach((cmd, idx) => {
        const item = document.createElement('div');
        item.className = `cmd-item ${idx === selectedCommandIndex ? 'selected' : ''}`;
        item.dataset.index = idx;

        item.innerHTML = `
            <div class="cmd-item-left">
                <span style="font-size: 15px; width: 20px; text-align: center;">${cmd.icon}</span>
                <span class="cmd-category-tag">${cmd.category}</span>
                <span style="font-weight: 500;">${cmd.title}</span>
            </div>
            ${cmd.hint ? `<span class="kbd-hint">${cmd.hint}</span>` : ''}
        `;

        item.onclick = () => {
            closeCommandPalette();
            cmd.handler();
        };

        item.onmouseenter = () => {
            selectedCommandIndex = idx;
            updateCommandSelection();
        };

        list.appendChild(item);
    });
}

function updateCommandSelection() {
    const items = document.querySelectorAll('.cmd-item');
    items.forEach((el, idx) => {
        if (idx === selectedCommandIndex) {
            el.classList.add('selected');
            el.scrollIntoView({ block: 'nearest' });
        } else {
            el.classList.remove('selected');
        }
    });
}

// Слушатели Command Palette
const cmdInput = document.getElementById('cmd-palette-input');
if (cmdInput) {
    cmdInput.addEventListener('input', (e) => {
        renderCommandPalette(e.target.value);
    });

    cmdInput.addEventListener('keydown', (e) => {
        if (e.key === 'ArrowDown') {
            e.preventDefault();
            if (filteredCommands.length > 0) {
                selectedCommandIndex = (selectedCommandIndex + 1) % filteredCommands.length;
                updateCommandSelection();
            }
        } else if (e.key === 'ArrowUp') {
            e.preventDefault();
            if (filteredCommands.length > 0) {
                selectedCommandIndex = (selectedCommandIndex - 1 + filteredCommands.length) % filteredCommands.length;
                updateCommandSelection();
            }
        } else if (e.key === 'Enter') {
            e.preventDefault();
            if (filteredCommands[selectedCommandIndex]) {
                const action = filteredCommands[selectedCommandIndex];
                closeCommandPalette();
                action.handler();
            }
        } else if (e.key === 'Escape') {
            e.preventDefault();
            closeCommandPalette();
        }
    });
}