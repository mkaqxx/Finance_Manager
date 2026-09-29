let categoryChartInstance = null;
let yearlyChartInstance = null;


async function loadAnalytics() {
    const monthInput = document.getElementById('analytics-month');
    const currencySelect = document.getElementById('analytics-currency');

    if (!monthInput.value) {
        const now = new Date();
        monthInput.value = `${now.getFullYear()}-${String(now.getMonth() + 1).padStart(2, '0')}`;
    }

    const selectedCurrency = currencySelect ? currencySelect.value : 'byn';
    const curSymbol = CURRENCY_SYMBOLS[selectedCurrency] || selectedCurrency.toUpperCase();

    const monthVal = monthInput.value;
    const [yearStr, monthStr] = monthVal.split('-');
    const year = parseInt(yearStr, 10);
    const month = parseInt(monthStr, 10);
    const firstDay = `${yearStr}-${monthStr}-01`;
    const lastDay = `${yearStr}-${monthStr}-${String(new Date(year, month, 0).getDate()).padStart(2, '0')}`;

    try {
        // C++ возвращает объекты со всеми валютами сразу
        const [catReport, yearlyReport, accounts] = await Promise.all([
            windowAPI.getCategoryReport(firstDay, lastDay),
            windowAPI.getYearlyReport(year),
            windowAPI.getAccounts()
        ]);

        const categoriesList = catReport[selectedCurrency] || [];
        const monthsList = yearlyReport[selectedCurrency] || [];

        // Данные текущего выбранного месяца
        const currentMonthData = monthsList[month - 1] || { income: 0, expense: 0 };
        document.getElementById('stat-income').innerText = `${currentMonthData.income.toLocaleString()} ${curSymbol}`;
        document.getElementById('stat-expense').innerText = `${currentMonthData.expense.toLocaleString()} ${curSymbol}`;

        // Баланс счетов только выбранной валюты
        const totalBalance = accounts
            .filter(acc => (acc.currency || '').toLowerCase() === selectedCurrency.toLowerCase())
            .reduce((sum, acc) => sum + acc.balance, 0);

        document.getElementById('stat-balance').innerText = `${totalBalance.toLocaleString()} ${curSymbol}`;

        // 1. Диаграмма структуры расходов
        const catCtx = document.getElementById('categoryChart').getContext('2d');
        if (categoryChartInstance) categoryChartInstance.destroy();

        const hasCategoryData = categoriesList.length > 0;

        categoryChartInstance = new Chart(catCtx, {
            type: 'doughnut',
            data: {
                labels: hasCategoryData ? categoriesList.map(c => c.name) : ['Нет расходов'],
                datasets: [{
                    data: hasCategoryData ? categoriesList.map(c => c.amount) : [1],
                    backgroundColor: hasCategoryData ? categoriesList.map(c => c.color || '#1db954') : ['#333333'],
                    borderWidth: 0
                }]
            },
            options: {
                plugins: {
                    legend: { position: 'bottom', labels: { color: '#fff' } }
                },
                cutout: '70%'
            }
        });

        // 2. Годовой график по месяцам
        const yearCtx = document.getElementById('yearlyChart').getContext('2d');
        if (yearlyChartInstance) yearlyChartInstance.destroy();

        yearlyChartInstance = new Chart(yearCtx, {
            type: 'bar',
            data: {
                labels: ['Янв', 'Фев', 'Мар', 'Апр', 'Май', 'Июн', 'Июл', 'Авг', 'Сен', 'Окт', 'Ноя', 'Дек'],
                datasets: [
                    {
                        label: `Доход (${curSymbol})`,
                        data: monthsList.map(m => m.income),
                        backgroundColor: '#1db954',
                        borderRadius: 4
                    },
                    {
                        label: `Расход (${curSymbol})`,
                        data: monthsList.map(m => m.expense),
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
                plugins: {
                    legend: { labels: { color: '#fff' } }
                }
            }
        });

    } catch (error) {
        console.error("Ошибка загрузки аналитики:", error);
    }
}