// --- ТРАНЗАКЦИИ ---
function toggleTxFields() {
    const type = document.getElementById('tx-type').value;
    document.getElementById('tx-category').style.display = type === 'transfer' ? 'none' : 'block';
    document.getElementById('tx-category').required = type !== 'transfer';
    document.getElementById('tx-dest-account').style.display = type === 'transfer' ? 'block' : 'none';
    document.getElementById('tx-dest-account').required = type === 'transfer';
    document.getElementById('tx-period').style.display = type === 'regular_expense' ? 'block' : 'none';
    document.getElementById('tx-period').required = type === 'regular_expense';
}

async function openTxModal() {
    document.getElementById('tx-modal').style.display = 'flex';
    document.getElementById('tx-form').reset();
    document.getElementById('tx-date').valueAsDate = new Date();
    toggleTxFields();

    const accSelect = document.getElementById('tx-account');
    const destSelect = document.getElementById('tx-dest-account');
    accSelect.innerHTML = '<option value="" disabled selected>Счет списания...</option>';
    destSelect.innerHTML = '<option value="" disabled selected>Счет зачисления...</option>';

    const accounts = await windowAPI.getAccounts();
    accounts.forEach(acc => {
        const opt = `<option value="${acc.id}">${acc.name} (${acc.balance} ${acc.currency})</option>`;
        accSelect.innerHTML += opt;
        destSelect.innerHTML += opt;
    });

    const catSelect = document.getElementById('tx-category');
    catSelect.innerHTML = '<option value="" disabled selected>Выберите категорию...</option>';
    for (const [id, cat] of Object.entries(categoriesMap)) {
        catSelect.innerHTML += `<option value="${id}">${cat.name}</option>`;
    }
}

function closeTxModal() { document.getElementById('tx-modal').style.display = 'none'; }

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
    if (type === 'transfer') data.destination_id = parseInt(document.getElementById('tx-dest-account').value);
    else if (type === 'regular_expense') data.period = document.getElementById('tx-period').value;

    try {
        await windowAPI.addTransaction(data);
        closeTxModal();
        if (document.getElementById('view-transactions').style.display === 'block') loadTransactions();
        if (document.getElementById('view-dashboard').style.display === 'block') loadDashboard();
    } catch (error) { alert(error.message); }
});

// --- СЧЕТА ---
function toggleAccountFields() {
    const type = document.getElementById('acc-type').value;
    document.getElementById('acc-bank-digits').style.display = type === 'bank_account' ? 'block' : 'none';
    document.getElementById('acc-bank-digits').required = type === 'bank_account';
    document.getElementById('acc-goal').style.display = type === 'savings_account' ? 'block' : 'none';
    document.getElementById('acc-goal').required = type === 'savings_account';
    document.getElementById('acc-deadline').style.display = type === 'savings_account' ? 'block' : 'none';
    document.getElementById('acc-deadline').required = type === 'savings_account';
}

function openAccountModal() {
    document.getElementById('account-modal').style.display = 'flex';
    document.getElementById('account-form').reset();
    document.getElementById('acc-deadline').valueAsDate = new Date();
    toggleAccountFields();
}

function closeAccountModal() { document.getElementById('account-modal').style.display = 'none'; }

document.getElementById('account-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const type = document.getElementById('acc-type').value;
    const data = {
        name: document.getElementById('acc-name').value,
        balance: parseFloat(document.getElementById('acc-balance').value),
        currency: document.getElementById('acc-currency').value,
        type: type
    };
    if (type === 'bank_account') data.last_four_digits = document.getElementById('acc-bank-digits').value;
    else if (type === 'savings_account') {
        data.goal_amount = parseFloat(document.getElementById('acc-goal').value);
        data.deadline = document.getElementById('acc-deadline').value;
    }

    try {
        await windowAPI.addAccount(data);
        closeAccountModal();
        if (document.getElementById('view-accounts').style.display === 'block') loadAccounts();
    } catch (error) { alert(error.message); }
});

// --- БЮДЖЕТЫ ---
function openBudgetModal() {
    document.getElementById('budget-modal').style.display = 'flex';
    document.getElementById('budget-form').reset();
    const catSelect = document.getElementById('budget-category');
    catSelect.innerHTML = '<option value="" disabled selected>Выберите категорию...</option>';
    for (const [id, cat] of Object.entries(categoriesMap)) {
        catSelect.innerHTML += `<option value="${id}">${cat.name}</option>`;
    }
}
function closeBudgetModal() { document.getElementById('budget-modal').style.display = 'none'; }

document.getElementById('budget-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    try {
        await windowAPI.addBudget({
            category_id: parseInt(document.getElementById('budget-category').value),
            limit: parseFloat(document.getElementById('budget-limit').value)
        });
        closeBudgetModal();
        if (document.getElementById('view-budget').style.display === 'block') renderBudgets();
    } catch (error) { alert(error.message); }
});

// --- КАТЕГОРИИ ---
function openCategoryModal() { document.getElementById('category-modal').style.display = 'flex'; document.getElementById('category-form').reset(); }
function closeCategoryModal() { document.getElementById('category-modal').style.display = 'none'; }

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
        if (document.getElementById('view-budget').style.display === 'block') renderCategories();
    } catch (error) { alert(error.message); }
});


function openAccountDetailModal(acc) {
    // показываем окно
    document.getElementById('account-detail-modal').style.display = 'flex';

    // заполняем название
    document.getElementById('modal-acc-name').textContent = acc.name;

    // заполняем информацию
    const sym = { 'rub': '₽', 'usd': '$', 'eur': '€', 'byn': 'Br' }[acc.currency] || acc.currency;
    let info = `
        <p>Баланс: <strong style="color:#fff">${acc.balance.toLocaleString()} ${sym}</strong></p>
        <p>Тип: ${acc.type}</p>
    `;
    if (acc.type === 'savings_account') {
        info += `
            <p>Цель: ${acc.goal.toLocaleString()} ${sym}</p>
            <p>Прогресс: ${acc.progress.toFixed(1)}%</p>
            <p>Нужно откладывать: ${acc.monthly_required.toLocaleString()} ${sym}/мес</p>
            <p>Дедлайн: ${acc.deadline}</p>
        `;
    }
    if (acc.type === 'bank_account') {
        info += `<p>Карта: •••• ${acc.number}</p>`;
    }
    document.getElementById('modal-acc-info').innerHTML = info;

    // загружаем транзакции
    loadAccountTransactions(acc.id);
}

function closeAccountDetailModal() {
    document.getElementById('account-detail-modal').style.display = 'none';
}

async function loadAccountTransactions(accountId) {
    const container = document.getElementById('modal-acc-transactions');
    container.innerHTML = '<span style="color:#b3b3b3">Загрузка...</span>';

    try {
        const transactions = await windowAPI.getTransactionsByAccount(accountId);
        container.innerHTML = '';

        if (!transactions.length) {
            container.innerHTML = '<span style="color:#b3b3b3">Транзакций нет</span>';
            return;
        }

        transactions.forEach(tx => {
            const cat = categoriesMap[tx.category_id] || { name: '-', color: '#fff' };
            const sign = tx.type === 'income' ? '+' : '-';
            container.innerHTML += `
                <div style="display:flex; justify-content:space-between; padding:10px; background:#282828; border-radius:6px;">
                    <span style="color:#b3b3b3">${tx.date}</span>
                    <span style="color:${cat.color}">${cat.name}</span>
                    <span style="font-weight:bold">${sign}${tx.amount.toLocaleString()} Br</span>
                </div>
            `;
        });
    }  catch (e) {
        console.error("Ошибка загрузки транзакций счёта:", e);
        container.innerHTML = '<span style="color:#e91429">Ошибка загрузки</span>';
    }
}