//экземпляры графиков chart.js
//старый график уничтожается через destroy() перед повторным созданием
let categoryChartInstance = null;
let yearlyChartInstance = null;

//получение цветов темы оформления для графиков
function getChartThemeColors() {
    //проверка темной темы
    const isDark = (document.documentElement.getAttribute('data-theme') || 'dark') === 'dark';
    //получение вычисленных стилей страницы
    const computed = getComputedStyle(document.documentElement);

    //считывание css-переменных темы
    const legendColor = computed.getPropertyValue('--text-title').trim() || (isDark ? '#ffffff' : '#2c2824');
    const tickColor = computed.getPropertyValue('--text-muted').trim() || (isDark ? '#737373' : '#8c8273');
    const gridColor = computed.getPropertyValue('--border-color').trim() || (isDark ? '#222222' : '#e2dbce');
    const incomeColor = computed.getPropertyValue('--color-income').trim() || (isDark ? '#00ff66' : '#5b8a72');
    const expenseColor = computed.getPropertyValue('--color-expense').trim() || (isDark ? '#ff0055' : '#c86d5e');
    const emptyColor = computed.getPropertyValue('--border-color').trim() || (isDark ? '#222222' : '#e2dbce');

    //возврат набора цветов
    return {
        legendColor,
        tickColor,
        gridColor,
        incomeColor,
        expenseColor,
        emptyColor
    };
}

//обновление цветов графиков при смене темы
function applyChartTheme() {
    const colors = getChartThemeColors();

    //обновление диаграммы категорий
    if (categoryChartInstance) {
        if (categoryChartInstance.options?.plugins?.legend?.labels) {
            categoryChartInstance.options.plugins.legend.labels.color = colors.legendColor;
        }
        if (categoryChartInstance.data?.datasets?.[0]?.data?.length === 1 && categoryChartInstance.data.labels?.[0] === 'Нет расходов') {
            categoryChartInstance.data.datasets[0].backgroundColor = [colors.emptyColor];
        }
        categoryChartInstance.update();
    }

    //обновление годового графика
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

//слушатель смены темы оформления
window.addEventListener('themeChanged', applyChartTheme);

//загрузка аналитики и построение графиков
async function loadAnalytics() {
    //поиск элементов выбора периода и валюты
    const monthInput = document.getElementById('analytics-month');
    const currencySelect = document.getElementById('analytics-currency');

    //установка текущего месяца по умолчанию если поле пустое
    if (!monthInput.value) {
        const now = new Date();
        monthInput.value = `${now.getFullYear()}-${String(now.getMonth() + 1).padStart(2, '0')}`;
    }

    //определение выбранной валюты и символа
    const selectedCurrency = currencySelect ? currencySelect.value : 'byn';
    const curSymbol = CURRENCY_SYMBOLS[selectedCurrency] || selectedCurrency.toUpperCase();

    //разбор даты на год и месяц
    const monthVal = monthInput.value;
    const [yearStr, monthStr] = monthVal.split('-');
    const year = parseInt(yearStr, 10);
    const month = parseInt(monthStr, 10);
    //расчет первого и последнего дней выбранного месяца
    const firstDay = `${yearStr}-${monthStr}-01`;
    const lastDay = `${yearStr}-${monthStr}-${String(new Date(year, month, 0).getDate()).padStart(2, '0')}`;

    try {
        //параллельный запрос отчетов и счетов из с++
        const [catReport, yearlyReport, accounts] = await Promise.all([
            windowAPI.getCategoryReport(firstDay, lastDay),
            windowAPI.getYearlyReport(year),
            windowAPI.getAccounts()
        ]);

        //выборка данных выбранной валюты
        const categoriesList = catReport[selectedCurrency] || [];
        const monthsList = yearlyReport[selectedCurrency] || [];

        //данные выбранного месяца
        const currentMonthData = monthsList[month - 1] || { income: 0, expense: 0 };

        //баланс счетов только выбранной валюты
        const totalBalance = (accounts || [])
            .filter(acc => (acc.currency || '').toLowerCase() === selectedCurrency.toLowerCase())
            .reduce((sum, acc) => sum + acc.balance, 0);

        //элементы карточек в шапке
        const incEl = document.getElementById('stat-income');
        const expEl = document.getElementById('stat-expense');
        const balEl = document.getElementById('stat-balance');

        //анимация счетчиков
        if (typeof animateValue === 'function') {
            animateValue(incEl, 0, currentMonthData.income, 700, v => `${Math.round(v).toLocaleString()} ${curSymbol}`);
            animateValue(expEl, 0, currentMonthData.expense, 700, v => `${Math.round(v).toLocaleString()} ${curSymbol}`);
            animateValue(balEl, 0, totalBalance, 700, v => `${Math.round(v).toLocaleString()} ${curSymbol}`);
        } else {
            incEl.innerText = `${currentMonthData.income.toLocaleString()} ${curSymbol}`;
            expEl.innerText = `${currentMonthData.expense.toLocaleString()} ${curSymbol}`;
            balEl.innerText = `${totalBalance.toLocaleString()} ${curSymbol}`;
        }

        //цвета текущей темы для canvas
        const themeColors = getChartThemeColors();

        //диаграмма расходов по категориям (doughnut)
        const catCtx = document.getElementById('categoryChart').getContext('2d');
        //уничтожение предыдущего экземпляра графика
        if (categoryChartInstance) categoryChartInstance.destroy();

        const hasCategoryData = categoriesList.length > 0;

        //создание круговой диаграммы
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
                animation: {
                    duration: 750,
                    easing: 'easeOutQuart'
                },
                plugins: {
                    legend: { position: 'bottom', labels: { color: themeColors.legendColor } }
                },
                cutout: '70%'
            }
        });

        //годовой график доходов и расходов по месяцам (bar)
        const yearCtx = document.getElementById('yearlyChart').getContext('2d');
        //уничтожение предыдущего экземпляра графика
        if (yearlyChartInstance) yearlyChartInstance.destroy();

        //создание столбчатой диаграммы
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
                animation: {
                    duration: 750,
                    easing: 'easeOutQuart'
                },
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