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
        <div id="accounts-list" style="display: flex; flex-direction: column; gap: 12px;">
            <div style="color: #b3b3b3;">Загрузка счетов...</div>
        </div>
    `;
        loadAccounts();
    } else if (page === 'transactions') {
        title.innerText = "История транзакций";
        content.innerHTML = `
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
        loadTransactions(); // Вызываем загрузку данных
    } else if (page === 'budget') {
        title.innerText = "Управление бюджетом";
        content.innerHTML = `<p style="color: #b3b3b3;">Здесь будут лимиты по категориям.</p>`;
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
                        <span style="font-weight: bold; color: #fff;">${cat.amount.toLocaleString()}</span>
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
            const sign = tx.type() === "income" ? '+' : '-';
            row.innerHTML = `
                <td style="padding: 12px 8px;">${tx.date}</td>   
                <td style="padding: 12px 8px; color: ${category.color};">
                    ${category.name}
                </td>
                <td style="padding: 12px 8px; font-weight: bold;">
                    ${sign}${tx.amount.toLocaleString()} ₽
                </td>
            `;
            tbody.appendChild(row);
        });
    }
    catch (error){
        console.error("Ошибка загрузки транзакций:", error);
    }
}


window.addEventListener('DOMContentLoaded', () => {
    loadCategories();
    switchPage('dashboard')
});