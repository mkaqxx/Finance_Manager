//словарь символов валют
const CURRENCY_SYMBOLS = {
    'byn': 'BYN',
    'usd': '$',
    'eur': '€',
    'rub': '₽'
};

//отрисовка скелетона для Bento-карточек во время загрузки
function renderDashboardSkeleton() {
    const skeletonLine = `<div class="skeleton skeleton-text" style="width: 60%; height: 20px; margin: 4px 0;"></div>`;
    const bVal = document.getElementById('balance-val');
    const eVal = document.getElementById('expenses-val');
    const iVal = document.getElementById('incomes-val');

    if (bVal && bVal.textContent.includes('Загрузка')) bVal.innerHTML = skeletonLine;
    if (eVal && eVal.textContent.includes('Загрузка')) eVal.innerHTML = skeletonLine;
    if (iVal && iVal.textContent.includes('Загрузка')) iVal.innerHTML = skeletonLine;
}

//получение набора закрытых в текущей сессии алертов
function getDismissedBudgetAlerts() {
    try {
        const stored = sessionStorage.getItem('dismissed_budget_alerts');
        return stored ? new Set(JSON.parse(stored)) : new Set();
    } catch {
        return new Set();
    }
}

//плавное скрытие баннера и сохранение в sessionStorage
function dismissBudgetAlert(alertKey, bannerElement) {
    const dismissed = getDismissedBudgetAlerts();
    dismissed.add(String(alertKey));
    try {
        sessionStorage.setItem('dismissed_budget_alerts', JSON.stringify(Array.from(dismissed)));
    } catch (e) {
        console.error('Ошибка сохранения закрытого алерта в sessionStorage:', e);
    }

    if (bannerElement) {
        bannerElement.style.transition = 'opacity 0.25s ease, max-height 0.3s ease, margin 0.3s ease, padding 0.3s ease';
        bannerElement.style.overflow = 'hidden';
        bannerElement.style.maxHeight = `${bannerElement.offsetHeight}px`;
        bannerElement.offsetHeight; // reflow
        bannerElement.style.opacity = '0';
        bannerElement.style.maxHeight = '0px';
        bannerElement.style.marginTop = '0px';
        bannerElement.style.marginBottom = '0px';
        bannerElement.style.paddingTop = '0px';
        bannerElement.style.paddingBottom = '0px';
        bannerElement.style.borderWidth = '0px';
        setTimeout(() => {
            bannerElement.remove();
        }, 300);
    }
}

//проверка бюджетов и вывод баннеров критического превышения (>100%)
function renderDashboardBudgetAlerts(budgets) {
    const alertContainer = document.getElementById('dashboard-budget-alerts');
    if (!alertContainer) return;
    alertContainer.innerHTML = '';

    if (!budgets || !budgets.length) return;

    const dismissed = getDismissedBudgetAlerts();

    // Показывать ТОЛЬКО критические перерасходы (>100% лимита).
    // Предупреждения 90% на Дашборде НЕ показывать (они отображаются внутри раздела «Бюджет»).
    const criticalBudgets = budgets.filter(b => {
        const isExceeded = b.status === 'exceeded' || (b.limit > 0 && b.current_amount > b.limit);
        const alertKey = String(b.category_id || b.id || b.name);
        return isExceeded && !dismissed.has(alertKey);
    });

    criticalBudgets.forEach(b => {
        const alertKey = String(b.category_id || b.id || b.name);
        const banner = document.createElement('div');
        banner.className = 'budget-alert-banner';

        const diffText = `Потрачено ${Number(b.current_amount).toLocaleString()} из ${Number(b.limit).toLocaleString()} BYN (перерасход ${(b.current_amount - b.limit).toLocaleString()} BYN)`;

        banner.innerHTML = `
            <div class="budget-alert-content">
                <span class="budget-alert-icon">⚠️</span>
                <div class="budget-alert-text">
                    <strong>Превышен бюджет по категории «${b.name}»!</strong>
                    <span>${diffText}</span>
                </div>
            </div>
            <div style="display: flex; align-items: center; gap: 8px;">
                <button type="button" onclick="switchPage('budget')" class="btn-secondary" style="padding: 6px 12px; font-size: 12.5px; white-space: nowrap;">
                    Управление лимитами →
                </button>
                <button type="button" class="btn-alert-close" title="Закрыть уведомление" aria-label="Закрыть">
                    ✕
                </button>
            </div>
        `;

        const closeBtn = banner.querySelector('.btn-alert-close');
        if (closeBtn) {
            closeBtn.onclick = (e) => {
                e.stopPropagation();
                dismissBudgetAlert(alertKey, banner);
            };
        }

        alertContainer.appendChild(banner);
    });
}

// Отрисовка динамического SVG-миниграфика (Sparkline) в Hero Bento
function renderHeroSparkline(currentBalance, netFlow, transactions = [], accounts = []) {
    const container = document.getElementById('hero-sparkline-svg');
    if (!container) return;

    const tagEl = document.getElementById('sparkline-tag');
    const cur = Number(currentBalance) || 0;

    // Счета в BYN для фильтрации транзакций
    const bynAccounts = new Set(
        (accounts || [])
            .filter(a => (a.currency || 'byn').toLowerCase() === 'byn')
            .map(a => a.id)
    );

    const now = new Date();
    const days = [];
    const dateKeys = [];
    const dailyDeltas = {};

    for (let i = 29; i >= 0; i--) {
        const d = new Date(now.getFullYear(), now.getMonth(), now.getDate() - i);
        const yyyy = d.getFullYear();
        const mm = String(d.getMonth() + 1).padStart(2, '0');
        const dd = String(d.getDate()).padStart(2, '0');
        const key = `${yyyy}-${mm}-${dd}`;
        dateKeys.push(key);
        dailyDeltas[key] = 0;
        days.push(d);
    }

    // Заполняем дельты по реальным транзакциям за каждый день
    (transactions || []).forEach(tx => {
        if (!tx.date) return;
        const txKey = String(tx.date).slice(0, 10);
        if (dailyDeltas[txKey] === undefined) return;

        const amt = Number(tx.amount) || 0;
        const isSrcByn = bynAccounts.size === 0 || bynAccounts.has(tx.account_id);
        const isDstByn = bynAccounts.size === 0 || bynAccounts.has(tx.destination_id);

        if (tx.type === 'income') {
            if (isSrcByn) dailyDeltas[txKey] += amt;
        } else if (tx.type === 'expense' || tx.type === 'regular_expense') {
            if (isSrcByn) dailyDeltas[txKey] -= amt;
        } else if (tx.type === 'transfer') {
            if (!isSrcByn && isDstByn) dailyDeltas[txKey] += amt;
            else if (isSrcByn && !isDstByn) dailyDeltas[txKey] -= amt;
        }
    });

    // Восстанавливаем подневный баланс от текущего назад
    const dailyBalances = new Array(30);
    dailyBalances[29] = cur;
    for (let i = 28; i >= 0; i--) {
        const nextKey = dateKeys[i + 1];
        dailyBalances[i] = dailyBalances[i + 1] - dailyDeltas[nextKey];
    }

    const startBalance = dailyBalances[0];
    const totalDelta = cur - startBalance;
    const isPositive = totalDelta >= 0;
    const strokeColor = isPositive ? 'var(--color-income)' : 'var(--color-expense)';
    const gradId = `spark-grad-${isPositive ? 'pos' : 'neg'}`;

    let minVal = Math.min(...dailyBalances);
    let maxVal = Math.max(...dailyBalances);
    if (minVal === maxVal) {
        minVal = Math.max(0, minVal * 0.95);
        maxVal = maxVal * 1.05 || 100;
    }
    const range = (maxVal - minVal) || 1;

    const width = 130;
    const height = 36;
    const padY = 4;
    const padX = 4;
    const actualH = height - padY * 2;
    const actualW = width - padX * 2;

    const coords = dailyBalances.map((val, idx) => {
        const x = padX + (idx / 29) * actualW;
        const normY = (val - minVal) / range;
        const y = height - padY - Math.min(Math.max(normY * actualH, 0), actualH);
        return {
            x: Number(x.toFixed(1)),
            y: Number(y.toFixed(1)),
            val,
            date: days[idx]
        };
    });

    let pathD = `M ${coords[0].x} ${coords[0].y}`;
    for (let i = 0; i < coords.length - 1; i++) {
        const p0 = coords[i];
        const p1 = coords[i + 1];
        const cpX = (p0.x + p1.x) / 2;
        pathD += ` C ${cpX} ${p0.y}, ${cpX} ${p1.y}, ${p1.x} ${p1.y}`;
    }

    const lastPt = coords[coords.length - 1];
    const areaD = `${pathD} L ${lastPt.x} ${height} L ${coords[0].x} ${height} Z`;

    const sign = totalDelta >= 0 ? '+' : '';
    const pct = startBalance > 0 ? ((totalDelta / startBalance) * 100).toFixed(1) : (totalDelta > 0 ? '+100' : '0');
    const wrap = document.getElementById('hero-sparkline-wrap');
    if (wrap) {
        wrap.title = `Динамика баланса за последние 30 дней: ${sign}${formatMoneyWithSpaces(totalDelta, 2)} BYN (${sign}${pct}%)\nСтарт (30 дн. назад): ${formatMoneyWithSpaces(startBalance, 2)} BYN\nСегодня: ${formatMoneyWithSpaces(cur, 2)} BYN`;
    }

    container.innerHTML = `
        <svg class="sparkline-svg" viewBox="0 0 ${width} ${height}" preserveAspectRatio="none">
            <defs>
                <linearGradient id="${gradId}" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="0%" stop-color="${strokeColor}" stop-opacity="0.32" />
                    <stop offset="100%" stop-color="${strokeColor}" stop-opacity="0.0" />
                </linearGradient>
            </defs>
            <path d="${areaD}" fill="url(#${gradId})" />
            <path class="sparkline-stroke" d="${pathD}" fill="none" stroke="${strokeColor}" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round" />
            <line class="sparkline-cursor-line" x1="${lastPt.x}" y1="2" x2="${lastPt.x}" y2="${height - 2}" stroke="var(--text-muted)" stroke-width="1" stroke-dasharray="2 2" stroke-opacity="0" />
            <circle class="sparkline-dot" cx="${lastPt.x}" cy="${lastPt.y}" r="3.5" fill="${strokeColor}" stroke="var(--bg-card)" stroke-width="1.5" />
            <rect width="${width}" height="${height}" fill="transparent" class="sparkline-hitbox" />
        </svg>
    `;

    // Интерактивное наведение мышью для просмотра баланса в любой из 30 дней
    const svgEl = container.querySelector('svg');
    const dot = container.querySelector('.sparkline-dot');
    const cursorLine = container.querySelector('.sparkline-cursor-line');
    const monthShort = ["янв", "фев", "мар", "апр", "май", "июн", "июл", "авг", "сен", "окт", "ноя", "дек"];

    if (svgEl && dot && tagEl) {
        svgEl.onmousemove = (e) => {
            const rect = svgEl.getBoundingClientRect();
            const mouseX = Math.max(0, Math.min(e.clientX - rect.left, rect.width));
            const normX = mouseX / rect.width;
            const ptIdx = Math.round(normX * (coords.length - 1));
            const pt = coords[ptIdx];
            if (!pt) return;

            dot.setAttribute('cx', pt.x);
            dot.setAttribute('cy', pt.y);
            if (cursorLine) {
                cursorLine.setAttribute('x1', pt.x);
                cursorLine.setAttribute('x2', pt.x);
                cursorLine.setAttribute('stroke-opacity', '0.5');
            }

            const d = pt.date;
            tagEl.textContent = `${d.getDate()} ${monthShort[d.getMonth()]}: ${formatMoneyWithSpaces(pt.val, 0)} BYN`;
            tagEl.style.color = 'var(--text-title)';
        };

        svgEl.onmouseleave = () => {
            dot.setAttribute('cx', lastPt.x);
            dot.setAttribute('cy', lastPt.y);
            if (cursorLine) {
                cursorLine.setAttribute('stroke-opacity', '0');
            }
            tagEl.textContent = 'Динамика 30 дней';
            tagEl.style.color = 'var(--text-muted)';
        };
    }
}

//отрисовка копилок и финансовых целей в Bento карточке
function renderDashboardSavingsGoals(accounts) {
    const container = document.getElementById('bento-savings-list');
    if (!container) return;
    container.innerHTML = '';

    const savings = (accounts || []).filter(acc => acc.type === 'savings_account');
    if (!savings.length) {
        container.innerHTML = `
            <div style="font-size: 12.5px; color: var(--text-muted); padding: 12px 4px; line-height: 1.5;">
                Активных копилок пока нет.<br>
                <a onclick="openAccountModal('savings_account')" style="color: var(--accent-color); cursor: pointer; font-weight: 600;">+ Создать цель</a>
            </div>
        `;
        return;
    }

    savings.forEach(acc => {
        const item = document.createElement('div');
        item.className = 'bento-savings-item';
        item.title = `Нажмите для просмотра счета «${acc.name}»`;
        item.onclick = () => openAccountDetailModal(acc);

        const goalVal = acc.goal || acc.goal_amount || 0;
        const cur = (acc.currency || 'byn').toLowerCase();
        const sym = CURRENCY_SYMBOLS[cur] || acc.currency.toUpperCase();

        const rawPct = goalVal > 0 ? (acc.balance / goalVal * 100) : (acc.progress || 0);
        const pct = Math.min(Math.max(rawPct, 0), 100);
        const isDone = rawPct >= 100;

        let deadlineStr = 'Бессрочно';
        if (acc.deadline) {
            const parts = String(acc.deadline).split('-');
            if (parts.length === 3) {
                deadlineStr = `до ${parts[2]}.${parts[1]}.${parts[0]}`;
            } else {
                deadlineStr = `до ${acc.deadline}`;
            }
        }

        item.innerHTML = `
            <div class="bento-savings-header">
                <span class="bento-savings-title">${acc.name}</span>
                <span class="bento-savings-pct" style="${isDone ? 'color: var(--color-income);' : ''}">${rawPct.toFixed(0)}%</span>
            </div>
            <div class="bento-savings-bar-track">
                <div class="bento-savings-bar-fill" style="width: 0%; ${isDone ? 'background: var(--color-income);' : ''}"></div>
            </div>
            <div class="bento-savings-meta">
                <span>${Number(acc.balance).toLocaleString()} из ${Number(goalVal).toLocaleString()} ${sym}</span>
                <span>📅 ${deadlineStr}</span>
            </div>
        `;

        container.appendChild(item);

        // Плавный выезд прогресс-бара от 0% до целевой ширины
        const barFill = item.querySelector('.bento-savings-bar-fill');
        if (barFill) {
            requestAnimationFrame(() => {
                requestAnimationFrame(() => {
                    barFill.style.width = `${pct}%`;
                });
            });
        }
    });
}

//загрузка данных за текущий месяц и заполнение Bento дашборда
async function loadDashboard() {
    try {
        renderDashboardSkeleton();

        //параллельный запрос отчета, бюджетов, счетов и транзакций из C++
        const [report, budgets, accounts, transactions] = await Promise.all([
            windowAPI.getMonthlyDashboard(),
            windowAPI.getBudgets().catch(() => []),
            windowAPI.getAccounts().catch(() => []),
            windowAPI.getTransactions().catch(() => [])
        ]);

        // 1. Отрисовка тревожных баннеров
        renderDashboardBudgetAlerts(budgets);

        // Извлечение основных валют
        const byn = report.byn || { balance: 0, expense: 0, income: 0, top_categories: [] };
        const usd = report.usd || { balance: 0, expense: 0, income: 0, top_categories: [] };
        const eur = report.eur || { balance: 0, expense: 0, income: 0, top_categories: [] };
        const rub = report.rub || { balance: 0, expense: 0, income: 0, top_categories: [] };

        // 2. HERO BENTO: Мульти-валютный общий баланс
        const balanceContainer = document.getElementById('balance-val');
        if (balanceContainer) {
            balanceContainer.innerHTML = `
                <div style="display: flex; flex-direction: column; gap: 4px;">
                    <div style="font-size: 32px; font-weight: 700; line-height: 1.1;" class="hero-amount-line">
                        <span class="anim-b split-amount" id="balance-num">0.00</span>
                        <span class="amount-currency" style="font-size: 18px; font-weight: 600; color: var(--text-muted); margin-left: 4px;">BYN</span>
                    </div>
                    <div style="display: flex; gap: 14px; font-size: 13px; color: var(--text-muted); font-weight: 500; margin-top: 4px;">
                        <span>$ <strong class="tab-num" style="color: var(--text-title);">${Number(usd.balance).toLocaleString()}</strong></span>
                        <span>€ <strong class="tab-num" style="color: var(--text-title);">${Number(eur.balance).toLocaleString()}</strong></span>
                        <span>₽ <strong class="tab-num" style="color: var(--text-title);">${Number(rub.balance).toLocaleString()}</strong></span>
                    </div>
                </div>
            `;
            const balNum = document.getElementById('balance-num');
            if (balNum) animateCountUp(balNum, Number(byn.balance), 800, '', 2);
        }

        // 3. HERO BENTO: Расчет чистого денежного потока (Net Cash Flow)
        const netFlow = byn.income - byn.expense;
        const netBadge = document.getElementById('net-flow-badge');
        if (netBadge) {
            const isPos = netFlow >= 0;
            netBadge.textContent = `${isPos ? '+' : ''}${formatMoneyWithSpaces(netFlow, 2)} BYN`;
            if (isPos) {
                netBadge.className = 'net-flow-badge';
            } else {
                netBadge.className = 'net-flow-badge negative';
            }
        }

        // Отрисовка динамики баланса (Sparkline) на основе реальных транзакций за 30 дней
        renderHeroSparkline(Number(byn.balance), netFlow, transactions, accounts);

        // 4. ДОХОД ЗА МЕСЯЦ
        const incEl = document.getElementById('incomes-val');
        if (incEl) {
            animateCountUp(incEl, Number(byn.income), 800, 'BYN', 2);
        }
        const incSub = document.getElementById('income-subtext');
        if (incSub) {
            const now = new Date();
            const monthNames = ["январь", "февраль", "март", "апрель", "май", "июнь", "июль", "август", "сентябрь", "октябрь", "ноябрь", "декабрь"];
            incSub.textContent = `Поступления за ${monthNames[now.getMonth()]}`;
        }

        // 5. РАСХОД ЗА МЕСЯЦ И СРЕДНИЙ РАСХОД В ДЕНЬ
        const expEl = document.getElementById('expenses-val');
        if (expEl) {
            animateCountUp(expEl, Number(byn.expense), 800, 'BYN', 2);
        }
        const expAvg = document.getElementById('expense-daily-avg');
        if (expAvg) {
            const now = new Date();
            const currentDay = Math.max(now.getDate(), 1);
            const dailyAvg = (byn.expense / currentDay).toFixed(1);
            expAvg.textContent = `~${dailyAvg} BYN / день (день ${currentDay})`;
        }

        // 6. КОПИЛКИ И ЦЕЛИ (4 КОЛОНКИ BENTO)
        renderDashboardSavingsGoals(accounts);

        // 7. СЧЕТА ПОЛЬЗОВАТЕЛЯ (мини-список счетов в Bento)
        const accountsContainer = document.getElementById('bento-accounts-list');
        if (accountsContainer) {
            accountsContainer.innerHTML = '';
            if (accounts && accounts.length) {
                // Выводим первые 4 счета
                accounts.slice(0, 4).forEach(acc => {
                    const item = document.createElement('div');
                    item.className = 'bento-mini-acc-item';
                    const icon = acc.type === 'bank_account' ? '💳' : acc.type === 'savings_account' ? '🎯' : '💵';
                    const sym = CURRENCY_SYMBOLS[(acc.currency || 'byn').toLowerCase()] || acc.currency;
                    
                    item.innerHTML = `
                        <div class="bento-mini-acc-left">
                            <span style="font-size: 15px;">${icon}</span>
                            <span class="bento-mini-acc-name">${acc.name}</span>
                        </div>
                        <span class="bento-mini-acc-balance amount">${Number(acc.balance).toLocaleString()} ${sym}</span>
                    `;
                    item.onclick = () => openAccountDetailModal(acc);
                    accountsContainer.appendChild(item);
                });
            } else {
                accountsContainer.innerHTML = `
                    <div style="font-size: 12.5px; color: var(--text-muted); padding: 8px 0;">
                        Счетов пока нет. <a onclick="openAccountModal()" style="color: var(--accent-color); cursor: pointer; font-weight: 600;">+ Добавить</a>
                    </div>
                `;
            }
        }

        // 8. ТОП КАТЕГОРИЙ РАСХОДОВ С БАРАМИ И БЕЙДЖАМИ
        const topContainer = document.getElementById('top-categories-list');
        const template = document.getElementById('tpl-top-category');
        if (topContainer) {
            topContainer.innerHTML = '';
            const allTop = [];
            const currencies = [
                { key: 'byn', label: 'BYN' },
                { key: 'usd', label: '$' },
                { key: 'eur', label: '€' },
                { key: 'rub', label: '₽' }
            ];

            currencies.forEach(curr => {
                const list = report[curr.key]?.top_categories || [];
                list.forEach(c => allTop.push({ ...c, currLabel: curr.label }));
            });

            if (allTop.length > 0) {
                // Находим максимальную сумму для масштабирования прогресс-баров
                const maxAmount = Math.max(...allTop.map(c => c.amount || 1));

                allTop.slice(0, 5).forEach(cat => {
                    const clone = template.content.cloneNode(true);
                    const dot = clone.querySelector('.tc-dot');
                    const nameEl = clone.querySelector('.tc-name');
                    const bar = clone.querySelector('.tc-bar');
                    const amountEl = clone.querySelector('.tc-amount');

                    const color = cat.color || 'var(--accent-color)';
                    if (dot) dot.style.backgroundColor = color;
                    if (nameEl) nameEl.textContent = cat.name;
                    if (amountEl) amountEl.textContent = `${cat.amount.toLocaleString()} ${cat.currLabel}`;

                    if (bar) {
                        const pct = Math.max(Math.round((cat.amount / maxAmount) * 100), 5);
                        bar.style.width = '0%';
                        bar.style.backgroundColor = color;
                        requestAnimationFrame(() => {
                            requestAnimationFrame(() => {
                                bar.style.width = `${pct}%`;
                            });
                        });
                    }

                    topContainer.appendChild(clone);
                });
            } else {
                topContainer.innerHTML = `
                    <div class="empty-state" style="padding: 24px 0; align-items: flex-start; text-align: left;">
                        <span style="color: var(--text-muted); font-size: 13px;">В этом месяце расходов пока нет.</span>
                    </div>
                `;
            }
        }

    } catch (error) {
        console.error("Ошибка загрузки дашборда:", error);
    }
}