let categoryChartInstance = null;
let yearlyChartInstance = null;


function getChartThemeColors() {
    const isDark = (document.documentElement.getAttribute('data-theme') || 'dark') === 'dark';
    const computed = getComputedStyle(document.documentElement);

    const legendColor = computed.getPropertyValue('--text-title').trim() || (isDark ? '#ffffff' : '#2c2824');
    const tickColor = computed.getPropertyValue('--text-muted').trim() || (isDark ? '#737373' : '#8c8273');
    const gridColor = computed.getPropertyValue('--border-color').trim() || (isDark ? '#222222' : '#e2dbce');
    const incomeColor = computed.getPropertyValue('--color-income').trim() || (isDark ? '#00ff66' : '#5b8a72');
    const expenseColor = computed.getPropertyValue('--color-expense').trim() || (isDark ? '#ff0055' : '#c86d5e');
    const emptyColor = computed.getPropertyValue('--border-color').trim() || (isDark ? '#222222' : '#e2dbce');

    return {
        legendColor,
        tickColor,
        gridColor,
        incomeColor,
        expenseColor,
        emptyColor
    };
}

function applyChartTheme() {
    const colors = getChartThemeColors();
    if (categoryChartInstance) {
        if (categoryChartInstance.options?.plugins?.legend?.labels) {
            categoryChartInstance.options.plugins.legend.labels.color = colors.legendColor;
        }
        if (categoryChartInstance.data?.datasets?.[0]?.data?.length === 1 && categoryChartInstance.data.labels?.[0] === 'Нет расходов') {
            categoryChartInstance.data.datasets[0].backgroundColor = [colors.emptyColor];
        }
        categoryChartInstance.update();
    }
    if (yearlyChartInstance) {
        if (yearlyChartInstance.options?.plugins?.legend?.labels) {
            yearlyChartInstance.options.plugins.legend.labels.color = colors.legendColor;
        }
        if (yearlyChartInstance.options?.scales?.x?.ticks) {
            yearlyChartInstance.options.scales.x.ticks.color = colors.tickColor;
        }
        if (yearlyChartInstance.options?.scales?.y?.ticks) {
            yearlyChartInstance.options.scales.y.ticks.color = colors.tickColor;
        }
        if (yearlyChartInstance.options?.scales?.y?.grid) {
            yearlyChartInstance.options.scales.y.grid.color = colors.gridColor;
        }
        if (yearlyChartInstance.data?.datasets?.[0]) {
            yearlyChartInstance.data.datasets[0].backgroundColor = colors.incomeColor;
        }
        if (yearlyChartInstance.data?.datasets?.[1]) {
            yearlyChartInstance.data.datasets[1].backgroundColor = colors.expenseColor;
        }
        yearlyChartInstance.update();
    }
}

window.addEventListener('themeChanged', applyChartTheme);

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

        const themeColors = getChartThemeColors();

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
                    backgroundColor: hasCategoryData ? categoriesList.map(c => c.color || themeColors.incomeColor) : [themeColors.emptyColor],
                    borderWidth: 0
                }]
            },
            options: {
                plugins: {
                    legend: { position: 'bottom', labels: { color: themeColors.legendColor } }
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
                        backgroundColor: themeColors.incomeColor,
                        borderRadius: 4
                    },
                    {
                        label: `Расход (${curSymbol})`,
                        data: monthsList.map(m => m.expense),
                        backgroundColor: themeColors.expenseColor,
                        borderRadius: 4
                    }
                ]
            },
            options: {
                responsive: true,
                scales: {
                    y: {
                        ticks: { color: themeColors.tickColor },
                        grid: { color: themeColors.gridColor }
                    },
                    x: {
                        ticks: { color: themeColors.tickColor },
                        grid: { display: false }
                    }
                },
                plugins: {
                    legend: { labels: { color: themeColors.legendColor } }
                }
            }
        });

    } catch (error) {
        console.error("Ошибка загрузки аналитики:", error);
    }
}