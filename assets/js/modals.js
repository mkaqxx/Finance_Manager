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

//открытие модального окна добавления транзакции
async function openTxModal() {
    //показ окна, сброс формы и установка текущей даты
    document.getElementById('tx-modal').style.display = 'flex';
    document.getElementById('tx-form').reset();
    document.getElementById('tx-date').valueAsDate = new Date();
    toggleTxFields();

    //очистка списков счетов
    const accSelect = document.getElementById('tx-account');
    const destSelect = document.getElementById('tx-dest-account');
    accSelect.innerHTML = '<option value="" disabled selected>Счет списания...</option>';
    destSelect.innerHTML = '<option value="" disabled selected>Счет зачисления...</option>';

    //запрос счетов из с++ бэкенда
    const accounts = await windowAPI.getAccounts();
    accounts.forEach(acc => {
        //безопасное добавление опций счетов в список
        const optText = `${acc.name} (${acc.balance} ${acc.currency})`;
        accSelect.appendChild(new Option(optText, acc.id));
        destSelect.appendChild(new Option(optText, acc.id));
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
                //безопасное добавление отфильтрованных счетов
                const optText = `${acc.name} (${acc.balance} ${acc.currency})`;
                destSelect.appendChild(new Option(optText, acc.id));
            });
    };

    //заполнение категорий из словаря
    const catSelect = document.getElementById('tx-category');
    catSelect.innerHTML = '<option value="" disabled selected>Выберите категорию...</option>';
    for (const [id, cat] of Object.entries(categoriesMap)) {
        //безопасное добавление категорий
        catSelect.appendChild(new Option(cat.name, id));
    }
}

//закрытие окна транзакций с анимацией
function closeTxModal() { 
    closeModalAnimated('tx-modal'); 
}

//обработка отправки формы транзакции
document.getElementById('tx-form').addEventListener('submit', async (e) => {
    //отмена перезагрузки страницы
    e.preventDefault();
    const type = document.getElementById('tx-type').value;

    const amountVal = parseFloat(document.getElementById('tx-amount').value);
    //проверка валидности суммы
    if (!Number.isFinite(amountVal) || amountVal <= 0) {
        if (typeof showToast === 'function') showToast('Пожалуйста, введите корректную сумму', 'error');
        return;
    }

    //сбор данных транзакции
    const data = {
        amount: amountVal,
        date: document.getElementById('tx-date').value,
        type: type,
        account_id: parseInt(document.getElementById('tx-account').value),
        category_id: type === 'transfer' ? 0 : parseInt(document.getElementById('tx-category').value)
    };

    //добавление получателя при переводе или периода при регулярном расходе
    if (type === 'transfer') {
        data.destination_id = parseInt(document.getElementById('tx-dest-account').value);
    } else if (type === 'regular_expense') {
        data.period = document.getElementById('tx-period').value;
    }

    try {
        //сохранение транзакции в с++ через api
        await windowAPI.addTransaction(data);
        //закрытие окна
        closeTxModal();

        //уведомление
        if (typeof showToast === 'function') showToast('Транзакция успешно сохранена', 'success');

        //конфетти при доходе
        if (type === 'income' && typeof triggerCelebrationConfetti === 'function') {
            triggerCelebrationConfetti();
        }

        //обновление открытых страниц
        if (document.getElementById('view-transactions').style.display === 'block') loadTransactions();
        if (document.getElementById('view-dashboard').style.display === 'block') loadDashboard();
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

    //номер карты для банковского счета
    document.getElementById('acc-bank-digits').style.display = type === 'bank_account' ? 'block' : 'none';
    document.getElementById('acc-bank-digits').required = type === 'bank_account';

    //цель и дедлайн для накопительного счета
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
        //режим редактирования счета
        editingAccountId = editData.id;
        title.textContent = 'Редактировать счет';
        submitBtn.textContent = 'Сохранить';

        document.getElementById('acc-name').value = editData.name;
        document.getElementById('acc-balance').value = editData.balance;
        currSelect.value = editData.currency;
        typeSelect.value = editData.type;

        //блокировка типа и валюты для существующего счета
        typeSelect.disabled = true;
        currSelect.disabled = true;

        if (editData.type === 'bank_account') {
            document.getElementById('acc-bank-digits').value = editData.number || '';
        } else if (editData.type === 'savings_account') {
            document.getElementById('acc-goal').value = editData.goal || '';
            document.getElementById('acc-deadline').value = editData.deadline || '';
        }
    } else {
        //режим добавления нового счета
        editingAccountId = null;
        title.textContent = 'Новый счет';
        submitBtn.textContent = 'Добавить';
        typeSelect.disabled = false;
        currSelect.disabled = false;
        document.getElementById('acc-deadline').valueAsDate = new Date();
    }

    //настройка видимости полей
    toggleAccountFields();
}

//закрытие формы счета с анимацией
function closeAccountModal() { 
    closeModalAnimated('account-modal'); 
}

//модальное окно детальной информации о счете
function openAccountDetailModal(acc) {
    //показ окна
    document.getElementById('account-detail-modal').style.display = 'flex';

    //установка названия счета
    document.getElementById('modal-acc-name').textContent = acc.name;

    //формирование информации о счете
    const sym = { 'rub': '₽', 'usd': '$', 'eur': '€', 'byn': 'Br' }[acc.currency] || acc.currency;
    let info = `
        <p>Баланс: <strong style="color: var(--text-title);">${acc.balance.toLocaleString()} ${sym}</strong></p>
        <p>Тип: ${acc.type}</p>
    `;

    //информация о цели для накопительного счета
    if (acc.type === 'savings_account') {
        const isReached = acc.progress >= 100;
        info += `
            <p>Цель: ${acc.goal.toLocaleString()} ${sym}</p>
            <p>Прогресс: ${acc.progress.toFixed(1)}% ${isReached ? '<span class="goal-achieved-badge">🎉 Цель достигнута!</span>' : ''}</p>
            <p>Нужно откладывать: ${acc.monthly_required.toLocaleString()} ${sym}/мес</p>
            <p>Дедлайн: ${acc.deadline}</p>
        `;
        //конфетти при достижении цели
        if (isReached && typeof triggerCelebrationConfetti === 'function') {
            setTimeout(() => triggerCelebrationConfetti(), 200);
        }
    }

    //информация о карте
    if (acc.type === 'bank_account') {
        info += `<p>Карта: •••• ${acc.number}</p>`;
    }
    document.getElementById('modal-acc-info').innerHTML = info;

    //загрузка истории операций счета
    loadAccountTransactions(acc.id);

    //кнопка удаления счета
    const deleteBtn = document.getElementById('btn-delete-account');
    if (deleteBtn) {
        deleteBtn.onclick = async () => {
            const ok = await showConfirm({
                title: 'Удаление счёта',
                message: `Удалить счёт "${acc.name}"? Все привязанные к нему транзакции также будут удалены.`,
                confirmText: 'Удалить'
            });
            if (!ok) return;

            try {
                //вызов удаления счета в с++
                await windowAPI.removeAccount(acc.id);
                if (typeof showToast === 'function') showToast(`Счёт "${acc.name}" удалён`, 'info');
                closeAccountDetailModal();
                loadAccounts();
                if (document.getElementById('view-dashboard').style.display === 'block') loadDashboard();
            } catch (e) {
                console.error("Ошибка удаления: " + e.message);
                if (typeof showToast === 'function') showToast(e.message, 'error');
            }
        };
    }

    //кнопка редактирования счета
    const editBtn = document.getElementById('btn-edit-account');
    if (editBtn) {
        editBtn.onclick = () => {
            closeAccountDetailModal();
            openAccountModal(acc);
        };
    }
}

//закрытие окна деталей счета
function closeAccountDetailModal() {
    closeModalAnimated('account-detail-modal');
}

//загрузка истории транзакций счета
async function loadAccountTransactions(accountId) {
    const container = document.getElementById('modal-acc-transactions');
    container.innerHTML = '<span style="color: var(--text-muted);">Загрузка...</span>';

    try {
        //запрос транзакций по счету из с++
        const transactions = await windowAPI.getTransactionsByAccount(accountId);
        container.innerHTML = '';

        //если транзакций нет, вывод заглушки
        if (!transactions.length) {
            container.innerHTML = '<span style="color: var(--text-muted);">Транзакций нет</span>';
            return;
        }

        //цикл по операциям счета
        transactions.forEach(tx => {
            const cat = categoriesMap[tx.category_id] || { name: '-', color: 'var(--text-title)' };
            const sign = tx.type === 'income' ? '+' : '-';
            const cur = (tx.currency || 'byn').toLowerCase();
            const sym = CURRENCY_SYMBOLS[cur] || cur.toUpperCase();

            //создание строки транзакции
            const row = document.createElement('div');
            row.style.cssText = 'display:flex; justify-content:space-between; padding:10px; background:var(--bg-input); border: 1px solid var(--border-color); border-radius:var(--card-radius);';

            //дата
            const dateSpan = document.createElement('span');
            dateSpan.style.color = 'var(--text-muted)';
            dateSpan.textContent = tx.date;

            //категория
            const catSpan = document.createElement('span');
            catSpan.style.color = cat.color || 'var(--text-title)';
            catSpan.textContent = cat.name;

            //сумма
            const amountSpan = document.createElement('span');
            amountSpan.style.fontWeight = 'bold';
            amountSpan.style.color = 'var(--text-title)';
            amountSpan.textContent = `${sign}${tx.amount.toLocaleString()}${sym}`;

            row.appendChild(dateSpan);
            row.appendChild(catSpan);
            row.appendChild(amountSpan);
            container.appendChild(row);
        });
    } catch (e) {
        console.error("Ошибка загрузки транзакций счёта:", e);
        container.innerHTML = '<span style="color: var(--color-expense);">Ошибка загрузки</span>';
    }
}

//обработка формы сохранения счета
document.getElementById('account-form').addEventListener('submit', async (e) => {
    e.preventDefault();

    const type = document.getElementById('acc-type').value;
    const nameVal = document.getElementById('acc-name').value.trim();
    const balanceVal = parseFloat(document.getElementById('acc-balance').value);

    //проверка заполнения имени и баланса счета
    if (!nameVal) {
        if (typeof showToast === 'function') showToast('Пожалуйста, введите название счёта', 'error');
        return;
    }
    if (!Number.isFinite(balanceVal)) {
        if (typeof showToast === 'function') showToast('Пожалуйста, введите корректный баланс', 'error');
        return;
    }

    const data = {
        name: nameVal,
        balance: balanceVal,
        currency: document.getElementById('acc-currency').value,
        type: type
    };

    //поля типа счета
    if (type === 'bank_account') {
        data.last_four_digits = document.getElementById('acc-bank-digits').value;
    } else if (type === 'savings_account') {
        const goalVal = parseFloat(document.getElementById('acc-goal').value);
        if (!Number.isFinite(goalVal) || goalVal <= 0) {
            if (typeof showToast === 'function') showToast('Пожалуйста, введите корректную сумму цели', 'error');
            return;
        }
        data.goal_amount = goalVal;
        data.deadline = document.getElementById('acc-deadline').value;
    }

    try {
        if (editingAccountId) {
            //редактирование счета
            data.id = editingAccountId;
            await windowAPI.editAccount(data);
            if (typeof showToast === 'function') showToast('Счёт успешно обновлён', 'success');
        } else {
            //добавление нового счета
            await windowAPI.addAccount(data);
            if (typeof showToast === 'function') showToast('Счёт успешно добавлен', 'success');
        }

        //закрытие окна и обновление интерфейса
        closeAccountModal();
        if (document.getElementById('view-accounts').style.display === 'block') loadAccounts();
        if (document.getElementById('account-detail-modal').style.display === 'flex') closeAccountDetailModal();
    } catch (error) {
        console.error("Ошибка сохранения:", error);
        if (typeof showToast === 'function') showToast(error.message, 'error');
    }
});


//хранит id категории редактируемого бюджета
let editingBudgetCategId = null;

//открытие модального окна бюджета
async function openBudgetModal(editData = null) {
    const modal = document.getElementById('budget-modal');
    const form = document.getElementById('budget-form');
    const title = modal.querySelector('h2');
    const submitBtn = modal.querySelector('button[type="submit"]');
    const catSelect = document.getElementById('budget-category');

    form.reset();

    if (editData) {
        //режим редактирования бюджета
        editingBudgetCategId = editData.category_id;
        title.textContent = 'Редактировать лимит';
        submitBtn.textContent = 'Сохранить';

        catSelect.innerHTML = `<option value="${editData.category_id}" selected>${editData.category_name || editData.name}</option>`;
        catSelect.disabled = true;
        document.getElementById('budget-limit').value = editData.limit;
    } else {
        //режим добавления нового лимита
        editingBudgetCategId = null;
        title.textContent = 'Установить лимит';
        submitBtn.textContent = 'Сохранить';
        catSelect.disabled = false;
        catSelect.innerHTML = '<option value="" disabled selected>Загрузка категорий...</option>';

        try {
            //запрос категорий из с++
            const categories = await windowAPI.getCategories();

            catSelect.innerHTML = '<option value="" disabled selected>Выберите категорию...</option>';

            //фильтрация только категорий расходов
            const expenseCategories = categories.filter(c => {
                const type = (c.type || '').toLowerCase();
                return type === 'expense';
            });

            const listToRender = expenseCategories.length > 0 ? expenseCategories : categories;

            listToRender.forEach(cat => {
                const opt = document.createElement('option');
                opt.value = cat.id;
                opt.textContent = cat.name;
                catSelect.appendChild(opt);
            });
        } catch (err) {
            console.error("Ошибка загрузки категорий в бюджет:", err);
            catSelect.innerHTML = '<option value="" disabled selected>Ошибка загрузки</option>';
        }
    }

    modal.style.display = 'flex';
}

//обработка отправки формы бюджета
document.getElementById('budget-form').onsubmit = async (e) => {
    e.preventDefault();

    const limitValue = parseFloat(document.getElementById('budget-limit').value);
    //проверка валидности лимита
    if (!Number.isFinite(limitValue) || limitValue <= 0) {
        if (typeof showToast === 'function') showToast('Пожалуйста, введите корректный лимит бюджета', 'error');
        return;
    }

    const catId = editingBudgetCategId !== null
        ? editingBudgetCategId
        : parseInt(document.getElementById('budget-category').value);

    const data = {
        category_id: Number(catId),
        limit: limitValue
    };

    try {
        if (editingBudgetCategId) {
            //вызов редактирования бюджета в с++
            await windowAPI.editBudget(data);
            if (typeof showToast === 'function') showToast('Лимит бюджета обновлён', 'success');
        } else {
            //добавление нового бюджета в с++
            await windowAPI.addBudget(data);
            if (typeof showToast === 'function') showToast('Лимит бюджета установлен', 'success');
        }

        //закрытие окна и перерисовка
        closeBudgetModal();
        renderBudgets();
    } catch (error) {
        if (typeof showToast === 'function') showToast(error.message, 'error');
        else alert("Ошибка: " + error.message);
    }
};

//закрытие окна бюджета
function closeBudgetModal() { 
    closeModalAnimated('budget-modal'); 
}


//открытие окна создания категории
function openCategoryModal() { 
    document.getElementById('category-modal').style.display = 'flex'; 
    document.getElementById('category-form').reset(); 
}

//закрытие окна категории с анимацией
function closeCategoryModal() { 
    closeModalAnimated('category-modal'); 
}

//обработка отправки формы категории
document.getElementById('category-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    try {
        //добавление категории в с++
        await windowAPI.addCategory({
            name: document.getElementById('cat-name').value,
            type: document.getElementById('cat-type').value,
            color: document.getElementById('cat-color').value
        });

        //закрытие окна и обновление кэша категорий
        closeCategoryModal();
        await loadCategories();

        if (typeof showToast === 'function') showToast('Категория успешно создана', 'success');
        if (document.getElementById('view-budget').style.display === 'block') renderCategories();
    } catch (error) {
        if (typeof showToast === 'function') showToast(error.message, 'error');
        else alert(error.message);
    }
});


//диалог подтверждения действия (возвращает promise)
function showConfirm({ title = 'Подтверждение', message = 'Вы уверены?', confirmText = 'Удалить', isDanger = true }) {
    return new Promise((resolve) => {
        //поиск элементов окна подтверждения
        const modal = document.getElementById('confirm-modal');
        const titleEl = document.getElementById('confirm-title');
        const msgEl = document.getElementById('confirm-message');
        const okBtn = document.getElementById('confirm-btn-ok');
        const cancelBtn = document.getElementById('confirm-btn-cancel');

        //заполнение текстов и цвета кнопки
        titleEl.textContent = title;
        msgEl.textContent = message;
        okBtn.textContent = confirmText;
        okBtn.style.background = isDanger ? 'var(--color-expense)' : 'var(--accent-color)';

        modal.style.display = 'flex';

        //очистка обработчиков и закрытие окна
        function cleanup(result) {
            closeModalAnimated(modal);
            okBtn.onclick = null;
            cancelBtn.onclick = null;
            resolve(result);
        }

        //клики: ок -> true, отмена -> false
        okBtn.onclick = () => cleanup(true);
        cancelBtn.onclick = () => cleanup(false);
    });
}