//отрисовка списка категорий
async function renderCategories() {
    try {
        const categories = await windowAPI.getCategories();
        const container = document.getElementById('categories-list');
        const template = document.getElementById('tpl-category-card');
        container.innerHTML = '';

        if (!categories || !categories.length) {
            container.innerHTML = `
                <div class="empty-state" style="grid-column: 1 / -1; padding: 32px 20px;">
                    <div class="empty-state-icon">
                        <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><path d="m4.93 4.93 4.24 4.24"/><circle cx="12" cy="12" r="4"/></svg>
                    </div>
                    <h4>Категории ещё не добавлены</h4>
                    <p>Создайте категории для удобной группировки расходов и доходов.</p>
                    <button type="button" onclick="openCategoryModal()" class="btn-primary" style="font-size: 13px;">+ Создать категорию</button>
                </div>
            `;
            return;
        }

        categories.forEach((cat) => {
            const clone = template.content.cloneNode(true);
            const dot = clone.querySelector('.cat-color-dot');
            if (dot) dot.style.backgroundColor = cat.color || 'var(--accent-color)';

            clone.querySelector('.cat-name').textContent = cat.name;
            clone.querySelector('.cat-type').textContent = cat.type === 'expense' ? 'Расход' : 'Доход';

            clone.querySelector('.cat-delete-btn').onclick = (e) => {
                const cardEl = e.target.closest('.card');
                handleRemoveCategory(cat, cardEl);
            };

            container.appendChild(clone);
        });
    } catch (e) {
        console.error("Ошибка отрисовки категорий:", e);
    }
}

//отрисовка карточек бюджетов
async function renderBudgets() {
    try {
        const budgets = await windowAPI.getBudgets();
        const container = document.getElementById('budget-list');
        const template = document.getElementById('tpl-budget-bar');
        container.innerHTML = '';

        if (!budgets || !budgets.length) {
            container.innerHTML = `
                <div class="empty-state" style="padding: 32px 20px;">
                    <div class="empty-state-icon">
                        <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2v20M17 5H9.5a3.5 3.5 0 0 0 0 7h5a3.5 3.5 0 0 1 0 7H6"/></svg>
                    </div>
                    <h4>Лимиты бюджета не установлены</h4>
                    <p>Установите месячные лимиты по категориям, чтобы контролировать перерасход средств.</p>
                    <button type="button" onclick="openBudgetModal()" class="btn-primary" style="font-size: 13px;">+ Установить лимит</button>
                </div>
            `;
            return;
        }

        budgets.forEach((b) => {
            const clone = template.content.cloneNode(true);

            const ratio = b.limit > 0 ? (b.current_amount / b.limit) : 0;
            const percent = b.limit > 0
                ? Math.min(ratio * 100, 999).toFixed(1)
                : (b.current_amount > 0 ? '100.0' : '0.0');

            let barColor = 'var(--color-income)';
            if (b.status === 'exceeded' || ratio >= 1.0) {
                barColor = 'var(--color-expense)';
            } else if (b.status === 'warning' || ratio >= 0.85) {
                barColor = 'var(--color-warning)';
            }

            const nameEl = clone.querySelector('.b-name');
            nameEl.textContent = b.name;
            if (b.color) nameEl.style.color = b.color;

            clone.querySelector('.b-current').textContent = Number(b.current_amount).toLocaleString();
            const curSym = (typeof CURRENCY_SYMBOLS !== 'undefined' && CURRENCY_SYMBOLS['byn']) ? CURRENCY_SYMBOLS['byn'] : 'BYN';
            clone.querySelector('.b-limit').textContent = `${Number(b.limit).toLocaleString()} ${curSym}`;
            clone.querySelector('.b-percent-text').textContent = percent;

            const bar = clone.querySelector('.b-progress-bar');
            bar.style.width = '0%';
            bar.style.backgroundColor = barColor;

            clone.querySelector('.bud-delete-btn').onclick = (e) => {
                const cardEl = e.target.closest('.card');
                handleRemoveBudget(b, cardEl);
            };

            clone.querySelector('.bud-edit-btn').onclick = () => openBudgetModal(b);

            container.appendChild(clone);

            // Плавный выезд шкалы прогресса от 0% до целевой ширины
            const targetWidth = `${Math.min(ratio * 100, 100)}%`;
            requestAnimationFrame(() => {
                requestAnimationFrame(() => {
                    bar.style.width = targetWidth;
                });
            });
        });
    } catch (e) {
        console.error("Ошибка отрисовки бюджетов:", e);
    }
}

//удаление категории с подтверждением
async function handleRemoveCategory(cat, cardEl) {
    try {
        const ok = await showConfirm({
            title: 'Удаление категории',
            message: `При удалении категории «${cat.name}» будут безвозвратно удалены все связанные с ней транзакции и лимиты. Продолжить?`,
            confirmText: 'Удалить'
        });
        if (!ok) return;

        if (cardEl) {
            cardEl.style.opacity = '0';
            cardEl.style.transform = 'scale(0.95)';
            await new Promise(r => setTimeout(r, 150));
        }

        await windowAPI.removeCategory(cat.id);
        if (typeof showToast === 'function') showToast(`Категория «${cat.name}» удалена`, 'info');

        await loadCategories();
        renderCategories();
        renderBudgets();
    } catch (e) {
        if (cardEl) {
            cardEl.style.opacity = '1';
            cardEl.style.transform = 'none';
        }
        console.error("Ошибка удаления категории:", e);
        if (typeof showToast === 'function') showToast(e.message, 'error');
    }
}

//сброс установленного лимита бюджета
async function handleRemoveBudget(b, cardEl) {
    try {
        const ok = await showConfirm({
            title: 'Сброс лимита',
            message: `Сбросить установленный лимит для категории «${b.name}»?`,
            confirmText: 'Сбросить'
        });
        if (!ok) return;

        if (cardEl) {
            cardEl.style.opacity = '0';
            cardEl.style.transform = 'scale(0.95)';
            await new Promise(r => setTimeout(r, 150));
        }

        await windowAPI.removeBudget(b.category_id);
        if (typeof showToast === 'function') showToast(`Лимит для «${b.name}» сброшен`, 'info');

        renderBudgets();
        if (document.getElementById('view-dashboard').style.display === 'block') loadDashboard();
    } catch (e) {
        if (cardEl) {
            cardEl.style.opacity = '1';
            cardEl.style.transform = 'none';
        }
        console.error("Ошибка сброса бюджета:", e);
        if (typeof showToast === 'function') showToast(e.message, 'error');
    }
}
