async function loadDashboard() {
    try {
        const report = await windowAPI.getMonthlyDashboard();

        document.getElementById('balance-val').innerText = `${report.balance.toLocaleString()} Br`;
        document.getElementById('expenses-val').innerText = `${report.total_expense.toLocaleString()} Br`;
        document.getElementById('incomes-val').innerText = `${report.total_income.toLocaleString()} Br`;

        const container = document.getElementById('top-categories-list');
        const template = document.getElementById('tpl-top-category');
        container.innerHTML = '';

        if (report.expenses_by_category && report.expenses_by_category.length > 0) {
            report.expenses_by_category.forEach(cat => {
                const clone = template.content.cloneNode(true);
                const nameEl = clone.querySelector('.tc-name');
                nameEl.textContent = cat.name;
                nameEl.style.color = cat.color;
                clone.querySelector('.tc-amount').textContent = `${cat.amount.toLocaleString()} Br`;
                container.appendChild(clone);
            });
        } else {
            container.innerHTML = '<span style="color: #b3b3b3;">В этом месяце расходов нет.</span>';
        }
    } catch (error) { console.error("Ошибка загрузки главной:", error); }
}