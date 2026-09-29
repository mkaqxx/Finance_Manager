const CURRENCY_SYMBOLS = {
    'byn': 'BYN',
    'usd': '$',
    'eur': '€',
    'rub': '₽'
};

async function loadDashboard() {
    try {
        const report = await windowAPI.getMonthlyDashboard();

        const byn = report.byn || { balance: 0, expense: 0, income: 0, top_categories: [] };
        const usd = report.usd || { balance: 0, expense: 0, income: 0, top_categories: [] };
        const eur = report.eur || { balance: 0, expense: 0, income: 0, top_categories: [] };
        const rub = report.rub || { balance: 0, expense: 0, income: 0, top_categories: [] };

        function formatMultiCurrency(b, u, e, r) {
            return `
                <div style="font-size: 16px; line-height: 1.5;">
                    <div>${b.toLocaleString()} <span style="font-size: 12px; color: #b3b3b3;">BYN</span></div>
                    <div>${u.toLocaleString()} <span style="font-size: 12px; color: #b3b3b3;">$</span></div>
                    <div>${e.toLocaleString()} <span style="font-size: 12px; color: #b3b3b3;">€</span></div>
                    <div>${r.toLocaleString()} <span style="font-size: 12px; color: #b3b3b3;">₽</span></div>
                </div>
            `;
        }

        // Общий баланс
        document.getElementById('balance-val').innerHTML = formatMultiCurrency(
            byn.balance, usd.balance, eur.balance, rub.balance
        );

        // Расходы за месяц
        document.getElementById('expenses-val').innerHTML = formatMultiCurrency(
            byn.expense, usd.expense, eur.expense, rub.expense
        );

        // Доходы за месяц
        document.getElementById('incomes-val').innerHTML = formatMultiCurrency(
            byn.income, usd.income, eur.income, rub.income
        );

        // Топ категорий по всем валютам
        const container = document.getElementById('top-categories-list');
        const template = document.getElementById('tpl-top-category');
        container.innerHTML = '';



        let hasAnyTop = false;
        const currencies = [
            { key: 'byn', label: 'BYN' },
            { key: 'usd', label: '$' },
            { key: 'eur', label: '€' },
            { key: 'rub', label: '₽' }
        ];
        currencies.forEach(curr => {
            const topList = report[curr.key]?.top_categories || [];
            if (topList.length > 0) {
                hasAnyTop = true;

                // Заголовок валютной группы
                const groupTitle = document.createElement('div');
                groupTitle.style.cssText = 'color: #b3b3b3; font-size: 12px; font-weight: bold; margin-top: 8px; text-transform: uppercase;';
                groupTitle.textContent = `Топ расходов (${curr.label})`;
                container.appendChild(groupTitle);

                topList.forEach(cat => {
                    const clone = template.content.cloneNode(true);
                    const nameEl = clone.querySelector('.tc-name');
                    nameEl.textContent = cat.name;
                    nameEl.style.color = cat.color || '#fff';
                    clone.querySelector('.tc-amount').textContent = `${cat.amount.toLocaleString()} ${curr.label}`;
                    container.appendChild(clone);
                });
            }
        });

        if (!hasAnyTop) {
            container.innerHTML = '<span style="color: #b3b3b3;">В этом месяце расходов нет.</span>';
        }
    } catch (error) {
        console.error("Ошибка загрузки главной:", error);
    }
}