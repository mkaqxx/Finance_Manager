const CURRENCY_SYMBOLS = {
    'byn': 'BYN',
    'usd': '$',
    'eur': '€',
    'rub': '₽'
};

function renderDashboardSkeleton() {
    const skeletonBox = `
        <div style="padding: 4px 0;">
            <div class="skeleton skeleton-text" style="width: 70%; height: 16px; margin-bottom: 6px;"></div>
            <div class="skeleton skeleton-text" style="width: 50%; height: 16px; margin-bottom: 6px;"></div>
            <div class="skeleton skeleton-text" style="width: 40%; height: 16px;"></div>
        </div>
    `;
    const bVal = document.getElementById('balance-val');
    const eVal = document.getElementById('expenses-val');
    const iVal = document.getElementById('incomes-val');
    if (bVal && bVal.textContent.includes('Загрузка')) bVal.innerHTML = skeletonBox;
    if (eVal && eVal.textContent.includes('Загрузка')) eVal.innerHTML = skeletonBox;
    if (iVal && iVal.textContent.includes('Загрузка')) iVal.innerHTML = skeletonBox;
}

function renderDashboardBudgetAlerts(budgets) {
    const alertContainer = document.getElementById('dashboard-budget-alerts');
    if (!alertContainer) return;
    alertContainer.innerHTML = '';

    if (!budgets || !budgets.length) return;

    // Ищем превышенные или близкие к исчерпанию лимиты (от 85%)
    const urgentBudgets = budgets.filter(b => {
        const ratio = b.limit > 0 ? (b.current_amount / b.limit) : 0;
        return b.status === 'exceeded' || ratio >= 0.85;
    });

    urgentBudgets.forEach(b => {
        const ratio = b.limit > 0 ? (b.current_amount / b.limit) : 0;
        const percent = (ratio * 100).toFixed(1);
        const isExceeded = b.status === 'exceeded' || ratio >= 1.0;
        const banner = document.createElement('div');
        banner.className = `budget-alert-banner ${isExceeded ? '' : 'warning'}`;

        const icon = isExceeded ? '⚠️' : '⚡';
        const title = isExceeded
            ? `Превышен бюджет по категории «${b.name}»!`
            : `Внимание: лимит «${b.name}» израсходован на ${percent}%`;
        const diffText = isExceeded
            ? `Потрачено ${b.current_amount.toLocaleString()} из ${b.limit.toLocaleString()} BYN (перерасход ${(b.current_amount - b.limit).toLocaleString()} BYN)`
            : `Потрачено ${b.current_amount.toLocaleString()} из ${b.limit.toLocaleString()} BYN (осталось ${(b.limit - b.current_amount).toLocaleString()} BYN)`;

        banner.innerHTML = `
            <div class="budget-alert-content">
                <span class="budget-alert-icon">${icon}</span>
                <div class="budget-alert-text">
                    <strong>${title}</strong>
                    <span>${diffText}</span>
                </div>
            </div>
            <button onclick="switchPage('budget')" class="btn-secondary" style="background: var(--bg-card); border: 1px solid var(--border-color); color: var(--text-main); border-radius: var(--card-radius); padding: 6px 14px; font-size: 13px; font-weight: 600; cursor: pointer; white-space: nowrap;">
                Управление лимитами →
            </button>
        `;
        alertContainer.appendChild(banner);
    });
}

async function loadDashboard() {
    try {
        renderDashboardSkeleton();
        const [report, budgets] = await Promise.all([
            windowAPI.getMonthlyDashboard(),
            windowAPI.getBudgets().catch(() => [])
        ]);

        // Отрисовываем алерты превышения лимитов
        renderDashboardBudgetAlerts(budgets);

        const byn = report.byn || { balance: 0, expense: 0, income: 0, top_categories: [] };
        const usd = report.usd || { balance: 0, expense: 0, income: 0, top_categories: [] };
        const eur = report.eur || { balance: 0, expense: 0, income: 0, top_categories: [] };
        const rub = report.rub || { balance: 0, expense: 0, income: 0, top_categories: [] };

        function renderAnimatedMultiCurrency(containerId, b, u, e, r) {
            const container = document.getElementById(containerId);
            if (!container) return;
            container.innerHTML = `
                <div style="font-size: 16px; line-height: 1.5;">
                    <div><span class="anim-num-b">0</span> <span style="font-size: 12px; color: var(--text-muted);">BYN</span></div>
                    <div><span class="anim-num-u">0</span> <span style="font-size: 12px; color: var(--text-muted);">$</span></div>
                    <div><span class="anim-num-e">0</span> <span style="font-size: 12px; color: var(--text-muted);">€</span></div>
                    <div><span class="anim-num-r">0</span> <span style="font-size: 12px; color: var(--text-muted);">₽</span></div>
                </div>
            `;
            if (typeof animateValue === 'function') {
                animateValue(container.querySelector('.anim-num-b'), 0, b, 700);
                animateValue(container.querySelector('.anim-num-u'), 0, u, 700);
                animateValue(container.querySelector('.anim-num-e'), 0, e, 700);
                animateValue(container.querySelector('.anim-num-r'), 0, r, 700);
            } else {
                container.querySelector('.anim-num-b').textContent = Number(b).toLocaleString();
                container.querySelector('.anim-num-u').textContent = Number(u).toLocaleString();
                container.querySelector('.anim-num-e').textContent = Number(e).toLocaleString();
                container.querySelector('.anim-num-r').textContent = Number(r).toLocaleString();
            }
        }

        // Общий баланс
        renderAnimatedMultiCurrency('balance-val', byn.balance, usd.balance, eur.balance, rub.balance);

        // Расходы за месяц
        renderAnimatedMultiCurrency('expenses-val', byn.expense, usd.expense, eur.expense, rub.expense);

        // Доходы за месяц
        renderAnimatedMultiCurrency('incomes-val', byn.income, usd.income, eur.income, rub.income);

        // Топ категорий по всем валютам
        const container = document.getElementById('top-categories-list');
        const template = document.getElementById('tpl-top-category');
        container.innerHTML = '';

        let hasAnyTop = false;
        let globalCatIdx = 0;
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
                groupTitle.style.cssText = 'color: var(--text-muted); font-size: 12px; font-weight: bold; margin-top: 8px; text-transform: uppercase;';
                groupTitle.textContent = `Топ расходов (${curr.label})`;
                container.appendChild(groupTitle);

                topList.forEach(cat => {
                    const clone = template.content.cloneNode(true);
                    const item = clone.querySelector('div');
                    if (item) {
                        item.classList.add('animate-cascade');
                        item.style.setProperty('--item-idx', globalCatIdx++);
                    }
                    const nameEl = clone.querySelector('.tc-name');
                    nameEl.textContent = cat.name;
                    nameEl.style.color = cat.color || 'var(--text-title)';

                    const dotEl = clone.querySelector('.tc-dot');
                    if (dotEl) {
                        dotEl.style.backgroundColor = cat.color || 'var(--accent-color)';
                    }

                    clone.querySelector('.tc-amount').textContent = `${cat.amount.toLocaleString()} ${curr.label}`;
                    container.appendChild(clone);
                });
            }
        });

        if (!hasAnyTop) {
            container.innerHTML = `
                <div class="empty-state" style="padding: 24px; text-align: left; align-items: flex-start;">
                    <span style="color: var(--text-muted); font-size: 14px;">В этом месяце расходов пока нет.</span>
                </div>
            `;
        }
    } catch (error) {
        console.error("Ошибка загрузки главной:", error);
    }
}