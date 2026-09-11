function switchPage(page, element) {
    const title = document.getElementById('page-title');
    const content = document.getElementById('page-content');

    // Убираем и добавляем класс active для меню
    document.querySelectorAll('.nav-links a').forEach(el => el.classList.remove('active'));
    if (element) {
        element.classList.add('active');
    }

    // Простая логика переключения «страниц» внутри SPA
    if (page === 'dashboard') {
        title.innerText = "Главная";
        content.innerHTML = `
        <div class="card-grid">
            <div class="card">
                <h3>Общий баланс</h3>
                <div class="amount" id="balance-val">Загрузка..</div>
            </div>
            <div class="card">
                <h3>Расходы за месяц</h3>
                <div class="amount" id="expenses-val" style="color: #e91429;">Загрузка..</div>
            </div>
            <div class="card">
                <h3>Доход за месяц</h3>
                <div class="amount" id="income-val" style="color: #1db954;">Загрузка..</div>
            </div>
        </div>
        
        <div style="margin-top: 32px;">
            <h3 style="margin-bottom: 16px; color: #fff;">Топ расходов за месяц</h3>
            <div id="top-categories-list" style="display: flex; flex-direction: column; gap: 10px;">
                <span style="color: #b3b3b3;">Загрузка...</span>
            </div>
        </div>`;

        loadDashboard();
    } else if (page === 'accounts') {
        title.innerText = "Мои счета";
        content.innerHTML = `
        <div style="display: flex; justify-content: space-between; margin-bottom: 16px;">
            <button onclick="openAccountModal()" style="padding: 8px 16px; background: #1db954; color: #000; font-weight: bold; border: none; border-radius: 4px; cursor: pointer;">+ Добавить счет</button>
        </div>
        <div id="accounts-list" style="display: flex; flex-direction: column; gap: 12px;">
            <div style="color: #b3b3b3;">Загрузка счетов...</div>
        </div>`;
        loadAccounts();
    } else if (page === 'transactions') {
        title.innerText = "История транзакций";
        content.innerHTML = `
            <div style="display: flex; justify-content: space-between; margin-bottom: 16px;">
                <button onclick="openTxModal()" style="padding: 8px 16px; background: #1db954; color: #000; font-weight: bold; border: none; border-radius: 4px; cursor: pointer;">+ Добавить</button>
            </div>
            <div style="background: #181818; border-radius: 8px; padding: 20px;">
                <table style="width: 100%; text-align: left; border-collapse: collapse;">
                    <thead>
                        <tr style="border-bottom: 1px solid #333; color: #b3b3b3;">
                            <th style="padding: 12px 8px;">Дата</th>
                            <th style="padding: 12px 8px;">Категория</th>
                            <th style="padding: 12px 8px;">Сумма</th>
                        </tr>
                    </thead>
                    <tbody id="transactions-table">
                        <tr><td colspan="3" style="padding: 12px 8px;">Загрузка...</td></tr>
                    </tbody>
                </table>
            </div>`;
        loadTransactions();
    } else if (page === 'budget') {
        title.innerText = "Управление бюджетом";
        content.innerHTML = `
    <div style="display: flex; justify-content: flex-start; gap: 12px; margin-bottom: 24px;">
        <button onclick="openCategoryModal()" style="padding: 8px 16px; background: #282828; color: #fff; border: 1px solid #333; border-radius: 4px; cursor: pointer;">+ Новая категория</button>
        <button onclick="openBudgetModal()" style="padding: 8px 16px; background: #1db954; color: #000; font-weight: bold; border: none; border-radius: 4px; cursor: pointer;">+ Установить лимит</button>
    </div>
    
    <h3 style="margin-bottom: 16px; color: #fff;">Мои категории</h3>
    <div id="categories-list" style="display: grid; grid-template-columns: repeat(auto-fill, minmax(200px, 1fr)); gap: 16px;"></div>

    <h3 style="margin-top: 32px; margin-bottom: 16px; color: #fff;">Бюджеты</h3>
    <div id="budget-list" style="display: flex; flex-direction: column; gap: 16px;"></div>`;

        renderCategories();
        renderBudgets();
    } else if (page === 'analytics') {
    title.innerText = "Аналитика";
    content.innerHTML = `
    <!-- Выбор периода -->
    <div style="display: flex; gap: 16px; margin-bottom: 24px;">
        <input type="month" id="analytics-month" style="padding: 10px; background: #282828; color: #fff; border: none; border-radius: 4px;">
        <button onclick="loadAnalytics()" style="padding: 10px 16px; background: #1db954; color: #000; font-weight: bold; border: none; border-radius: 4px; cursor: pointer;">Применить</button>
    </div>
    
    <!-- Сводка за месяц -->
    <div class="card-grid" style="margin-bottom: 24px;">
        <div class="card">
            <h3>Доход за месяц</h3>
            <div class="amount" id="stat-income" style="color: #1db954;">Загрузка..</div>
        </div>
        <div class="card">
            <h3>Расходы за месяц</h3>
            <div class="amount" id="stat-expense" style="color: #e91429;">Загрузка..</div>
        </div>
        <div class="card">
            <h3>Общий баланс</h3>
            <div class="amount" id="stat-balance">Загрузка..</div>
        </div>
    </div>

    <!-- Графики -->
    <div style="display: grid; grid-template-columns: 1fr 2fr; gap: 20px;">
        <div style="background: #181818; padding: 20px; border-radius: 8px; display: flex; flex-direction: column; align-items: center;">
            <h3 style="margin-bottom: 16px; color: #fff; align-self: flex-start;">Структура расходов</h3>
            <div style="width: 100%; max-width: 250px;">
                <canvas id="categoryChart"></canvas>
            </div>
        </div>
        <div style="background: #181818; padding: 20px; border-radius: 8px;">
            <h3 style="margin-bottom: 16px; color: #fff;">Годовой отчет</h3>
            <canvas id="yearlyChart"></canvas>
        </div>
    </div>`;

    // Устанавливаем текущий месяц по умолчанию
    const now = new Date();
    const currentMonth = `${now.getFullYear()}-${String(now.getMonth() + 1).padStart(2, '0')}`;
    document.getElementById('analytics-month').value = currentMonth;

    loadAnalytics();
}
}


async function loadDashboard() {
    try {
        const report = await window.getMonthlyDashboard();

        // 1. Заполняем главные карточки
        document.getElementById('balance-val').innerText = report.balance.toLocaleString();
        document.getElementById('expenses-val').innerText = report.total_expense.toLocaleString();
        document.getElementById('income-val').innerText = report.total_income.toLocaleString();

        // 2. Отрисовываем топ категорий
        const topContainer = document.getElementById('top-categories-list');
        topContainer.innerHTML = '';

        if (report.expenses_by_category && report.expenses_by_category.length > 0) {
            report.expenses_by_category.forEach(cat => {
                topContainer.innerHTML += `
                    <div style="display: flex; justify-content: space-between; padding: 14px 16px; background: #181818; border: 1px solid #282828; border-radius: 8px;">
                        <span style="color: ${cat.color}; font-weight: bold;">${cat.name}</span>
                        <span style="font-weight: bold; color: #fff;">${cat.amount.toLocaleString()} BYN</span>
                    </div>
                `;
            });
        } else {
            topContainer.innerHTML = '<span style="color: #b3b3b3;">В этом месяце расходов нет.</span>';
        }

    } catch (error) {
        console.error("Ошибка загрузки отчета:", error);
    }
}


async function loadAccounts() {
    try {
        const accounts = await window.getAccounts();
        const container = document.getElementById('accounts-list');
        if (!container) return;

        container.innerHTML = ''; // Очищаем строку "Загрузка..."

        // Словарь символов валют
        const currencySymbols = {
            'rub': '₽',
            'usd': '$',
            'eur': '€',
            'byn': 'Br'
        };

        accounts.forEach(acc => {
            const card = document.createElement('div');
            card.style.cssText = `
                background: #181818;
                border-radius: 8px;
                padding: 16px 20px;
                cursor: pointer;
                transition: background 0.2s ease;
                border: 1px solid #282828;
            `;

            // Определяем символ валюты или используем код, если символа нет в словаре
            const sym = currencySymbols[acc.currency.toLowerCase()] || acc.currency.toUpperCase();

            // Формируем дополнительную информацию в зависимости от типа счета
            const extraInfo = acc.type === 'bank_account' && acc.number
                ? `•••• ${acc.number}`
                : acc.type === 'savings_account' && acc.goal
                    ? `Цель: ${acc.goal.toLocaleString()} ${sym} (Прогресс: ${acc.progress.toFixed(1)}%)`
                    : 'Наличные';

            // Собираем HTML карточки
            card.innerHTML = `
                <div style="display: flex; justify-content: space-between; align-items: center;">
                    <div>
                        <div style="font-weight: bold; font-size: 16px; color: #fff;">${acc.name}</div>
                        <div style="color: #b3b3b3; font-size: 12px; margin-top: 4px;">
                            ${acc.currency.toUpperCase()} • ${extraInfo}
                        </div>
                    </div>
                    <div style="font-size: 18px; font-weight: bold; color: #1db954;">
                        ${acc.balance.toLocaleString()} ${sym}
                    </div>
                </div>

                <!-- Скрытый блок с деталями -->
                <div class="acc-details" style="display: none; margin-top: 16px; padding-top: 16px; border-top: 1px solid #282828; color: #b3b3b3; font-size: 14px;">
                    <p style="margin: 4px 0;"><strong>ID счета:</strong> ${acc.id}</p>
                    <p style="margin: 4px 0;"><strong>Тип:</strong> ${acc.type}</p>
                    <div style="margin-top: 12px; display: flex; gap: 8px;">
                        <button onclick="event.stopPropagation(); alert('Скоро: редактирование')" 
                                style="background: #282828; color: #fff; border: none; padding: 6px 12px; border-radius: 4px; cursor: pointer;">
                            Редактировать
                        </button>
                    </div>
                </div>
            `;

            card.addEventListener('click', () => {
                const details = card.querySelector('.acc-details');
                const isHidden = details.style.display === 'none';

                document.querySelectorAll('.acc-details').forEach(el => el.style.display = 'none');

                if (isHidden) {
                    details.style.display = 'block';
                }
            });

            // Эффекты наведения
            card.addEventListener('mouseenter', () => card.style.background = '#222222');
            card.addEventListener('mouseleave', () => card.style.background = '#181818');

            container.appendChild(card);
        });
    } catch (error) {
        console.error("Ошибка загрузки счетов:", error);
    }
}


let categoriesMap = {};

async function loadCategories() {
    try {
        const data = await window.getCategories();
        data.forEach(cat => {
            // Сохраняем объект целиком
            categoriesMap[cat.id] = {
                name: cat.name,
                color: cat.color
            };
        });
    } catch (error) {
        console.error("Ошибка загрузки категорий:", error);
    }
}

async function loadTransactions(){
    try{
        const data = await window.getTransactions();
        const tbody = document.getElementById('transactions-table')
        tbody.innerHTML = '';
        data.forEach(tx =>{
            const row = document.createElement('tr');
            row.style.borderBottom = "1px solid #282828";
            const category = categoriesMap[tx.category_id] || { name: "Неизвестно", color: "#ffffff" };
            const sign = tx.type === "income" ? '+' : '-';
            row.innerHTML = `
                <td style="padding: 12px 8px;">${tx.date}</td>   
                <td style="padding: 12px 8px; color: ${category.color};">
                    ${category.name}
                </td>
                <td style="padding: 12px 8px; font-weight: bold;">
                    ${sign}${tx.amount.toLocaleString()} BYN
                </td>
            `;
            tbody.appendChild(row);
        });
    }
    catch (error){
        console.error("Ошибка загрузки транзакций:", error);
    }
}


function toggleTxFields() {
    const type = document.getElementById('tx-type').value;
    const catSelect = document.getElementById('tx-category');
    const destSelect = document.getElementById('tx-dest-account');
    const periodSelect = document.getElementById('tx-period');

    // Сбрасываем видимость и обязательность
    catSelect.style.display = type === 'transfer' ? 'none' : 'block';
    catSelect.required = type !== 'transfer';

    destSelect.style.display = type === 'transfer' ? 'block' : 'none';
    destSelect.required = type === 'transfer';

    periodSelect.style.display = type === 'regular_expense' ? 'block' : 'none';
    periodSelect.required = type === 'regular_expense';
}

async function openTxModal() {
    document.getElementById('tx-modal').style.display = 'flex';
    document.getElementById('tx-amount').value = '';
    document.getElementById('tx-date').valueAsDate = new Date();

    // Запускаем переключатель, чтобы форма приняла правильный вид
    toggleTxFields();
    // Заполняем счета
    const accSelect = document.getElementById('tx-account');
    const destSelect = document.getElementById('tx-dest-account'); // Для перевода

    accSelect.innerHTML = '<option value="" disabled selected>Счет списания...</option>';
    destSelect.innerHTML = '<option value="" disabled selected>Счет зачисления...</option>';

    const accounts = await window.getAccounts();
    accounts.forEach(acc => {
        const optionHTML = `<option value="${acc.id}">${acc.name} (${acc.balance} ${acc.currency})</option>`;
        accSelect.innerHTML += optionHTML;
        destSelect.innerHTML += optionHTML;
    });

    // Заполняем категории из кэша
    const catSelect = document.getElementById('tx-category');
    catSelect.innerHTML = '<option value="" disabled selected>Выберите категорию...</option>';
    for (const [id, cat] of Object.entries(categoriesMap)) {
        catSelect.innerHTML += `<option value="${id}">${cat.name}</option>`;
    }
}


function closeTxModal() {
    document.getElementById('tx-modal').style.display = 'none';
    document.getElementById('tx-form').reset();
}


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

    const responseStr = await window.addTransaction(data);
    const response = typeof responseStr  === 'string' ? JSON.parse(responseStr) : responseStr;
    if (response.error) {
        alert(response.error);
        return;
    }
    closeTxModal();
    loadTransactions();
});


function toggleAccountFields() {
    const type = document.getElementById('acc-type').value;
    const digits = document.getElementById('acc-bank-digits');
    const goal = document.getElementById('acc-goal');
    const deadline = document.getElementById('acc-deadline');

    digits.style.display = type === 'bank_account' ? 'block' : 'none';
    digits.required = type === 'bank_account';

    goal.style.display = type === 'savings_account' ? 'block' : 'none';
    goal.required = type === 'savings_account';

    deadline.style.display = type === 'savings_account' ? 'block' : 'none';
    deadline.required = type === 'savings_account';
}

function openAccountModal() {
    document.getElementById('account-modal').style.display = 'flex';
    document.getElementById('account-form').reset();
    document.getElementById('acc-deadline').valueAsDate = new Date();
    toggleAccountFields();
}

function closeAccountModal() {
    document.getElementById('account-modal').style.display = 'none';
}

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

    await window.addAccount(data);
    closeAccountModal();
    loadAccounts(); // Обновляем список счетов
});


async function renderCategories() {
    const container = document.getElementById('categories-list');
    if (!container) return;
    try {
        const categories = await window.getCategories();
        container.innerHTML = categories.length ? '' : '<span style="color: #b3b3b3;">Нет категорий.</span>';
        categories.forEach(cat => {
            const typeLabel = cat.type === 'expense' ? 'Расход' : 'Доход';
            container.innerHTML += `
                <div style="background: #181818; border: 1px solid #282828; border-radius: 8px; padding: 16px; display: flex; justify-content: space-between; align-items: center;">
                    <div style="display: flex; align-items: center; gap: 12px;">
                        <div style="width: 16px; height: 16px; border-radius: 50%; background-color: ${cat.color};"></div>
                        <div>
                            <div style="font-weight: bold; color: #fff; font-size: 15px;">${cat.name}</div>
                            <div style="font-size: 12px; color: #b3b3b3; margin-top: 4px;">${typeLabel}</div>
                        </div>
                    </div>
                    <button onclick="alert('Удаление ID: ${cat.id}')" style="background: transparent; color: #e91429; border: none; cursor: pointer; font-size: 18px;">×</button>
                </div>`;
        });
    } catch (e) { console.error(e); }
}


async function renderBudgets() {
    const container = document.getElementById('budget-list');
    if (!container) return;
    try {
        const budgets = await window.getBudgets();
        container.innerHTML = budgets.length ? '' : '<span style="color: #b3b3b3;">Лимиты не установлены.</span>';

        budgets.forEach(b => {
            const percent = Math.min((b.current_amount / b.limit) * 100, 100).toFixed(1);
            let barColor = '#1db954'; // normal
            if (b.status === 'warning') barColor = '#f39c12';
            if (b.status === 'exceeded') barColor = '#e91429';

            container.innerHTML += `
                <div style="background: #181818; border: 1px solid #282828; border-radius: 8px; padding: 16px;">
                    <div style="display: flex; justify-content: space-between; margin-bottom: 8px;">
                        <span style="font-weight: bold; color: ${b.color};">${b.name}</span>
                        <span style="color: #b3b3b3;">
                            <span style="color: #fff;">${b.current_amount.toLocaleString()}</span> / ${b.limit.toLocaleString()} BYN (${percent}%)
                        </span>
                    </div>
                    <div style="width: 100%; height: 8px; background: #333; border-radius: 4px; overflow: hidden;">
                        <div style="width: ${percent}%; height: 100%; background: ${barColor}; transition: width 0.3s;"></div>
                    </div>
                </div>`;
        });
    } catch (e) { console.error(e); }
}


async function openBudgetModal() {
    document.getElementById('budget-modal').style.display = 'flex';
    document.getElementById('budget-form').reset();
    const catSelect = document.getElementById('budget-category');
    catSelect.innerHTML = '<option value="" disabled selected>Выберите категорию...</option>';
    for (const [id, cat] of Object.entries(categoriesMap)) {
        catSelect.innerHTML += `<option value="${id}">${cat.name}</option>`;
    }
}
function closeBudgetModal() { document.getElementById('budget-modal').style.display = 'none'; }


async function openCategoryModal(){
    document.getElementById('category-modal').style.display = 'flex';
    document.getElementById('category-form').reset();
}

function  closeCategoryModal(){
    document.getElementById('category-modal').style.display = 'none';
}



document.getElementById('budget-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const data = {
        category_id: parseInt(document.getElementById('budget-category').value),
        limit: parseFloat(document.getElementById('budget-limit').value)
    };
    try {
        await window.addBudget(data);
        closeBudgetModal();
        if (document.getElementById('budget-list')) renderBudgets();
    } catch (error) { console.error(error); }
});


document.getElementById('category-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const data = {
        name: document.getElementById('cat-name').value,
        type: document.getElementById('cat-type').value,
        color: document.getElementById('cat-color').value
    };

    try {
        const rawResponse = await window.addCategory(data);
        const response = typeof rawResponse === 'string' ? JSON.parse(rawResponse) : rawResponse;

        if (response.error) {
            alert(response.error);
            return;
        }

        closeCategoryModal();
        await loadCategories(); // Обновляем глобальный кэш categoriesMap для транзакций

        // Сразу перерисовываем сетку, если мы на вкладке "Бюджет"
        if (document.getElementById('categories-list')) {
            renderCategories();
        }
    } catch (error) {
        console.error("Ошибка при добавлении категории:", error);
    }
});


// Глобальные переменные для хранения графиков (чтобы их можно было обновлять)
let categoryChartInstance = null;
let yearlyChartInstance = null;

async function loadAnalytics() {
    const monthVal = document.getElementById('analytics-month').value; // формат "YYYY-MM"
    if (!monthVal) return;

    const [yearStr, monthStr] = monthVal.split('-');
    const year = parseInt(yearStr);
    const month = parseInt(monthStr);

    // Высчитываем первый и последний день месяца (ГГГГ-ММ-ДД)
    const firstDay = `${yearStr}-${monthStr}-01`;
    const lastDayDate = new Date(year, month, 0);
    const lastDay = `${yearStr}-${monthStr}-${String(lastDayDate.getDate()).padStart(2, '0')}`;

    try {
        // 1. Запрашиваем отчеты из C++
        const catReportStr = await window.getCategoryReport([firstDay, lastDay]);
        const catReport = typeof catReportStr === 'string' ? JSON.parse(catReportStr) : catReportStr;

        const yearlyReportStr = await window.getYearlyReport([year]);
        const yearlyReport = typeof yearlyReportStr === 'string' ? JSON.parse(yearlyReportStr) : yearlyReportStr;

        // 2. Обновляем сводные карточки
        const currentMonthData = yearlyReport.months[month - 1];
        document.getElementById('stat-income').innerText = currentMonthData.income.toLocaleString() + ' ₽';
        document.getElementById('stat-expense').innerText = currentMonthData.expense.toLocaleString() + ' ₽';

        // Баланс высчитываем "на лету" из счетов
        const accounts = await window.getAccounts();
        const totalBalance = accounts.reduce((sum, acc) => sum + acc.balance, 0);
        document.getElementById('stat-balance').innerText = totalBalance.toLocaleString() + ' ₽';

        // 3. Отрисовка круговой диаграммы (Категории)
        const catCtx = document.getElementById('categoryChart').getContext('2d');
        if (categoryChartInstance) categoryChartInstance.destroy();

        const catLabels = catReport.expenses_by_category.map(c => c.name);
        const catData = catReport.expenses_by_category.map(c => c.amount);
        const catColors = catReport.expenses_by_category.map(c => c.color);

        categoryChartInstance = new Chart(catCtx, {
            type: 'doughnut',
            data: {
                labels: catLabels.length ? catLabels : ['Нет данных'],
                datasets: [{
                    data: catData.length ? catData : [1],
                    backgroundColor: catColors.length ? catColors : ['#333333'],
                    borderWidth: 0
                }]
            },
            options: {
                plugins: { legend: { position: 'bottom', labels: { color: '#fff' } } },
                cutout: '70%'
            }
        });

        // 4. Отрисовка столбчатой диаграммы (Годовой отчет)
        const yearCtx = document.getElementById('yearlyChart').getContext('2d');
        if (yearlyChartInstance) yearlyChartInstance.destroy();

        const monthNames = ['Янв', 'Фев', 'Мар', 'Апр', 'Май', 'Июн', 'Июл', 'Авг', 'Сен', 'Окт', 'Ноя', 'Дек'];
        const incomes = yearlyReport.months.map(m => m.income);
        const expenses = yearlyReport.months.map(m => m.expense);

        yearlyChartInstance = new Chart(yearCtx, {
            type: 'bar',
            data: {
                labels: monthNames,
                datasets: [
                    {
                        label: 'Доход',
                        data: incomes,
                        backgroundColor: '#1db954',
                        borderRadius: 4
                    },
                    {
                        label: 'Расход',
                        data: expenses,
                        backgroundColor: '#e91429',
                        borderRadius: 4
                    }
                ]
            },
            options: {
                responsive: true,
                scales: {
                    y: { ticks: { color: '#b3b3b3' }, grid: { color: '#333' } },
                    x: { ticks: { color: '#b3b3b3' }, grid: { display: false } }
                },
                plugins: { legend: { labels: { color: '#fff' } } }
            }
        });

    } catch (error) {
        console.error("Ошибка при загрузке аналитики:", error);
    }
}


window.addEventListener('DOMContentLoaded', () => {
    loadCategories();
    switchPage('dashboard')
});