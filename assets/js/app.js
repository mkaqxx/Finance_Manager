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
                        <div class = "card">
                        <h3>Общий баланс</h3>
                        <div class = "ammount" id = "balance-val">Загрузка...</div>
                        </div>
                        <div class ="card">
                        <h3><Расходы за месяц/h3>
                        <div class = "amount" id = "expenses-val" style="color: #e91429;">Загрузка...</div>
                        </div>
                    </div>`;
        loadFinanceData();
    } else if (page === 'accounts') {
        title.innerText = "Ваши счета";
        content.innerHTML = `<p style="color: #b3b3b3;">Здесь будет список банковских карт и наличных.</p>`;
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


async function loadFinanceData() {
    try {
        if (typeof window.getFinanceData === 'function') {
            const data = await window.getFinanceData();

            // Обновляем цифры на экране
            document.getElementById('balance-val').innerText = data.balance + ' ₽';
            document.getElementById('expenses-val').innerText = data.expenses + ' ₽';
            document.getElementById('incomes-val').innerText = data.incomes + ' ₽'
        }
    } catch (error) {
        console.error("Ошибка при получении данных из C++:", error);
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
            const color = tx.type() === "income" ? '#1db954' : '#e91429';
            const sign = tx.type() === "income" ? '+' : '';
            row.innerHTML = `
                <td style="padding: 12px 8px;">${tx.date}</td>
                <td style="padding: 12px 8px;">${tx.category}</td>
                <td style="padding: 12px 8px; color: ${color}; font-weight: bold;">
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
    loadFinanceData();
});