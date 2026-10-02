let editingBudgetCategId = null;

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
                        <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><path d="m4.93 4.93 4.24 4.24"/><circle cx="12" cy="12" r="4"/></svg>
                    </div>
                    <h4>Категории ещё не добавлены</h4>
                    <p>Создайте категории для удобной группировки расходов и доходов.</p>
                    <button onclick="openCategoryModal()" class="btn-primary" style="font-size: 13px;">+ Создать категорию</button>
                </div>
            `;
            return;
        }

        categories.forEach((cat, idx) => {
            const clone = template.content.cloneNode(true);
            const card = clone.querySelector('.card');
            if (card) {
                card.classList.add('animate-cascade');
                card.style.setProperty('--item-idx', idx);
            }
            clone.querySelector('.cat-color-dot').style.backgroundColor = cat.color;
            clone.querySelector('.cat-name').textContent = cat.name;
            clone.querySelector('.cat-type').textContent = cat.type === 'expense' ? 'Расход' : 'Доход';

            // Вызов функции удаления категории с анимацией
            clone.querySelector('.cat-delete-btn').onclick = (e) => {
                const cardEl = e.target.closest('.card');
                handleRemoveCategory(cat, cardEl);
            };

            container.appendChild(clone);
        });
    } catch (e) { console.error(e); }
}

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
                        <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2v20M17 5H9.5a3.5 3.5 0 0 0 0 7h5a3.5 3.5 0 0 1 0 7H6"/></svg>
                    </div>
                    <h4>Лимиты бюджета не установлены</h4>
                    <p>Установите месячные лимиты по категориям, чтобы контролировать перерасход средств.</p>
                    <button onclick="openBudgetModal()" class="btn-primary" style="font-size: 13px;">+ Установить лимит</button>
                </div>
            `;
            return;
        }

        budgets.forEach((b, idx) => {
            const clone = template.content.cloneNode(true);
            const card = clone.querySelector('.card');
            if (card) {
                card.classList.add('animate-cascade');
                card.style.setProperty('--item-idx', idx);
                if (b.status === 'exceeded') {
                    card.classList.add('budget-exceeded-pulse');
                }
            }
            const percent = Math.min((b.current_amount / b.limit) * 100, 100).toFixed(1);
            let barColor = b.status === 'exceeded' ? 'var(--color-expense)' : b.status === 'warning' ? '#f39c12' : 'var(--color-income)';

            const nameEl = clone.querySelector('.b-name');
            nameEl.textContent = b.name;
            nameEl.style.color = b.color;
            clone.querySelector('.b-current').textContent = b.current_amount.toLocaleString();
            clone.querySelector('.b-limit').textContent = `${b.limit.toLocaleString()}${CURRENCY_SYMBOLS['byn']}`;
            clone.querySelector('.b-percent-text').textContent = percent;

            const bar = clone.querySelector('.b-progress-bar');
            bar.style.width = `${percent}%`;
            bar.style.backgroundColor = barColor;

            // Вызов функции удаления бюджета с анимацией
            clone.querySelector('.bud-delete-btn').onclick = (e) => {
                const cardEl = e.target.closest('.card');
                handleRemoveBudget(b, cardEl);
            };
            clone.querySelector('.bud-edit-btn').onclick = () => openBudgetModal(b);

            container.appendChild(clone);
        });
    } catch (e) { console.error(e); }
}

// === ФУНКЦИИ УДАЛЕНИЯ (с плавным схлопыванием) ===

async function handleRemoveCategory(cat, cardEl) {
    try {
        const ok = await showConfirm({
            title: 'Удаление категории',
            message: `При удалении категории "${cat.name}" будут безвозвратно удалены все связанные с ней транзакции и лимиты. Продолжить?`,
            confirmText: 'Да, удалить'
        });
        if (!ok) return;

        if (cardEl) {
            cardEl.classList.add('row-deleting');
            await new Promise(r => setTimeout(r, 260));
        }

        await windowAPI.removeCategory(cat.id);
        if (typeof showToast === 'function') showToast(`Категория "${cat.name}" удалена`, 'info');
        await loadCategories();
        renderCategories();
        renderBudgets();
    } catch (e) {
        if (cardEl) cardEl.classList.remove('row-deleting');
        console.error("Ошибка удаления: " + e.message);
        if (typeof showToast === 'function') showToast(e.message, 'error');
    }
}

async function handleRemoveBudget(b, cardEl) {
    try {
        const ok = await showConfirm({
            title: 'Удаление лимита',
            message: `Сбросить установленный лимит для "${b.name}"?`,
            confirmText: 'Сбросить'
        });
        if (!ok) return;

        if (cardEl) {
            cardEl.classList.add('row-deleting');
            await new Promise(r => setTimeout(r, 260));
        }

        await windowAPI.removeBudget(b.category_id);
        if (typeof showToast === 'function') showToast(`Лимит для "${b.name}" сброшен`, 'info');
        renderBudgets();
    } catch (e) {
        if (cardEl) cardEl.classList.remove('row-deleting');
        console.error("Ошибка удаления: " + e.message);
        if (typeof showToast === 'function') showToast(e.message, 'error');
    }
}


