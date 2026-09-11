let categoryChartInstance = null;
let yearlyChartInstance = null;

async function loadAnalytics() {
    const monthInput = document.getElementById('analytics-month');
    if (!monthInput.value) {
        const now = new Date();
        monthInput.value = `${now.getFullYear()}-${String(now.getMonth() + 1).padStart(2, '0')}`;
    }
    const monthVal = monthInput.value;
    const [yearStr, monthStr] = monthVal.split('-');
    const year = parseInt(yearStr), month = parseInt(monthStr);
    const firstDay = `${yearStr}-${monthStr}-01`;
    const lastDay = `${yearStr}-${monthStr}-${String(new Date(year, month, 0).getDate()).padStart(2, '0')}`;

    try {
        const catReport = await windowAPI.getCategoryReport(firstDay, lastDay);
        const yearlyReport = await windowAPI.getYearlyReport(year);

        const currentMonthData = yearlyReport.months[month - 1];
        document.getElementById('stat-income').innerText = `${currentMonthData.income.toLocaleString()} Br`;
        document.getElementById('stat-expense').innerText = `${currentMonthData.expense.toLocaleString()} Br`;

        const accounts = await windowAPI.getAccounts();
        const totalBalance = accounts.reduce((sum, acc) => sum + acc.balance, 0);
        document.getElementById('stat-balance').innerText = `${totalBalance.toLocaleString()} Br`;

        const catCtx = document.getElementById('categoryChart').getContext('2d');
        if (categoryChartInstance) categoryChartInstance.destroy();
        categoryChartInstance = new Chart(catCtx, {
            type: 'doughnut',
            data: {
                labels: catReport.expenses_by_category.length ? catReport.expenses_by_category.map(c => c.name) : ['Нет данных'],
                datasets: [{
                    data: catReport.expenses_by_category.length ? catReport.expenses_by_category.map(c => c.amount) : [1],
                    backgroundColor: catReport.expenses_by_category.length ? catReport.expenses_by_category.map(c => c.color) : ['#333333'],
                    borderWidth: 0
                }]
            },
            options: { plugins: { legend: { position: 'bottom', labels: { color: '#fff' } } }, cutout: '70%' }
        });

        const yearCtx = document.getElementById('yearlyChart').getContext('2d');
        if (yearlyChartInstance) yearlyChartInstance.destroy();
        yearlyChartInstance = new Chart(yearCtx, {
            type: 'bar',
            data: {
                labels: ['Янв', 'Фев', 'Мар', 'Апр', 'Май', 'Июн', 'Июл', 'Авг', 'Сен', 'Окт', 'Ноя', 'Дек'],
                datasets: [
                    { label: 'Доход', data: yearlyReport.months.map(m => m.income), backgroundColor: '#1db954', borderRadius: 4 },
                    { label: 'Расход', data: yearlyReport.months.map(m => m.expense), backgroundColor: '#e91429', borderRadius: 4 }
                ]
            },
            options: { responsive: true, scales: { y: { ticks: { color: '#b3b3b3' }, grid: { color: '#333' } }, x: { ticks: { color: '#b3b3b3' }, grid: { display: false } } }, plugins: { legend: { labels: { color: '#fff' } } } }
        });
    } catch (error) { console.error("Ошибка аналитики:", error); }
}