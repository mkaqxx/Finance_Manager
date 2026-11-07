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
    const legendColor = computed.getPropertyValue('--text-title').trim() || (isDark ? '#f4f4f5' : '#18181b');
    const tickColor = computed.getPropertyValue('--text-muted').trim() || (isDark ? '#a1a1aa' : '#71717a');
    const gridColor = computed.getPropertyValue('--border-color').trim() || (isDark ? 'rgba(255,255,255,0.08)' : 'rgba(0,0,0,0.08)');
    const incomeColor = computed.getPropertyValue('--color-income').trim() || (isDark ? '#10b981' : '#059669');
    const expenseColor = computed.getPropertyValue('--color-expense').trim() || (isDark ? '#f43f5e' : '#e11d48');
    const emptyColor = computed.getPropertyValue('--border-color').trim() || (isDark ? '#27272a' : '#e4ded4');

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

        //анимация счетчиков (CountUp с 2 знаками и разделителями)
        if (typeof animateCountUp === 'function') {
            animateCountUp(incEl, currentMonthData.income, 800, curSymbol, 2);
            animateCountUp(expEl, currentMonthData.expense, 800, curSymbol, 2);
            animateCountUp(balEl, totalBalance, 800, curSymbol, 2);
        } else if (typeof animateValue === 'function') {
            animateValue(incEl, 0, currentMonthData.income, 800, v => `${Math.round(v).toLocaleString()} ${curSymbol}`);
            animateValue(expEl, 0, currentMonthData.expense, 800, v => `${Math.round(v).toLocaleString()} ${curSymbol}`);
            animateValue(balEl, 0, totalBalance, 800, v => `${Math.round(v).toLocaleString()} ${curSymbol}`);
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

        //создание круговой диаграммы с плавной анимацией вращения и масштабирования
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
                responsive: true,
                maintainAspectRatio: true,
                animation: {
                    animateRotate: true,
                    animateScale: true,
                    duration: 900,
                    easing: 'easeOutCubic'
                },
                plugins: {
                    legend: {
                        display: false // Скрываем встроенную легенду: используем двухколоночную HTML-сетку
                    },
                    tooltip: {
                        callbacks: {
                            label: function(context) {
                                const val = context.parsed || 0;
                                const total = context.dataset.data.reduce((a, b) => a + b, 0);
                                const pct = total > 0 ? ((val / total) * 100).toFixed(1) : 0;
                                return ` ${context.label}: ${Number(val).toLocaleString()} ${curSymbol} (${pct}%)`;
                            }
                        }
                    }
                },
                cutout: '70%'
            }
        });

        //отрисовка адаптивной двухколоночной сетки элементов легенды
        const legendContainer = document.getElementById('category-chart-legend');
        if (legendContainer) {
            legendContainer.innerHTML = '';
            if (hasCategoryData) {
                const totalExpense = categoriesList.reduce((sum, c) => sum + (c.amount || 0), 0);
                categoriesList.forEach((cat, idx) => {
                    const item = document.createElement('div');
                    item.className = 'category-legend-item';
                    item.title = `${cat.name}: ${Number(cat.amount).toLocaleString()} ${curSymbol} (нажмите, чтобы скрыть/показать)`;

                    const color = cat.color || themeColors.incomeColor;
                    const pctVal = cat.percent !== undefined 
                        ? Number(cat.percent).toFixed(1) 
                        : (totalExpense > 0 ? ((cat.amount / totalExpense) * 100).toFixed(1) : '0.0');

                    item.innerHTML = `
                        <span class="category-legend-dot" style="background-color: ${color};"></span>
                        <span class="category-legend-name" title="${cat.name}">${cat.name}</span>
                        <span class="category-legend-pct">${pctVal}%</span>
                    `;

                    item.onclick = () => {
                        const isVisible = categoryChartInstance.getDataVisibility(idx);
                        categoryChartInstance.toggleDataVisibility(idx);
                        categoryChartInstance.update();
                        item.classList.toggle('hidden-cat', isVisible);
                    };

                    legendContainer.appendChild(item);
                });
            } else {
                legendContainer.innerHTML = `
                    <div style="grid-column: 1 / -1; text-align: center; font-size: 12px; color: var(--text-muted); padding: 8px 0;">
                        Нет расходов за выбранный период
                    </div>
                `;
            }
        }

        //годовой график доходов и расходов по месяцам (bar)
        const yearCtx = document.getElementById('yearlyChart').getContext('2d');
        //уничтожение предыдущего экземпляра графика
        if (yearlyChartInstance) yearlyChartInstance.destroy();

        //создание столбчатой диаграммы с плавной каскадной анимацией роста столбиков
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
                animations: {
                    y: {
                        easing: 'easeOutQuart',
                        duration: 800,
                        from: (ctx) => {
                            if (ctx.type === 'data') {
                                return ctx.chart.scales.y.getPixelForValue(0);
                            }
                        },
                        delay: (ctx) => {
                            return ctx.type === 'data' ? ctx.dataIndex * 35 : 0;
                        }
                    }
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