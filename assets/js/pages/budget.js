let editingBudgetCategId = null;

async function renderCategories() {
    try {
        const categories = await windowAPI.getCategories();
        const container = document.getElementById('categories-list');
        const template = document.getElementById('tpl-category-card');
        container.innerHTML = categories.length ? '' : '<span style="color: var(--text-muted);">Нет категорий.</span>';

        categories.forEach(cat => {
            const clone = template.content.cloneNode(true);
            clone.querySelector('.cat-color-dot').style.backgroundColor = cat.color;
            clone.querySelector('.cat-name').textContent = cat.name;
            clone.querySelector('.cat-type').textContent = cat.type === 'expense' ? 'Расход' : 'Доход';

            // Вызов функции удаления категории
            clone.querySelector('.cat-delete-btn').onclick = () => handleRemoveCategory(cat);

            container.appendChild(clone);
        });
    } catch (e) { console.error(e); }
}

async function renderBudgets() {
    try {
        const budgets = await windowAPI.getBudgets();
        const container = document.getElementById('budget-list');
        const template = document.getElementById('tpl-budget-bar');
        container.innerHTML = budgets.length ? '' : '<span style="color: var(--text-muted);">Лимиты не установлены.</span>';

        budgets.forEach(b => {
            const clone = template.content.cloneNode(true);
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

            // ИСПРАВЛЕНО: передаем b.category_id вместо b.id
            clone.querySelector('.bud-delete-btn').onclick = () => handleRemoveBudget(b);
            clone.querySelector('.bud-edit-btn').onclick = () => openBudgetModal(b);

            container.appendChild(clone);
        });
    } catch (e) { console.error(e); }
}

// === ФУНКЦИИ УДАЛЕНИЯ (обязательно должны быть здесь) ===

async function handleRemoveCategory(cat) {
    try {
        const ok = await showConfirm({
            title: 'Удаление категории',
            message: `При удалении категории "${cat.name}" будут безвозвратно удалены все связанные с ней транзакции и лимиты. Продолжить?`,
            confirmText: 'Да, удалить'
        });
        if (!ok) return;
        await windowAPI.removeCategory(cat.id);
        if (typeof showToast === 'function') showToast(`Категория "${cat.name}" удалена`, 'info');
        await loadCategories();
        renderCategories();
        renderBudgets();
    } catch (e) {
        console.error("Ошибка удаления: " + e.message);
        if (typeof showToast === 'function') showToast(e.message, 'error');
    }
}

async function handleRemoveBudget(b) {
    try {
        const ok = await showConfirm({
            title: 'Удаление лимита',
            message: `Сбросить установленный лимит для "${b.name}"?`,
            confirmText: 'Сбросить'
        });
        if (!ok) return;

        await windowAPI.removeBudget(b.category_id);
        if (typeof showToast === 'function') showToast(`Лимит для "${b.name}" сброшен`, 'info');
        renderBudgets();
    } catch (e) {
        console.error("Ошибка удаления: " + e.message);
        if (typeof showToast === 'function') showToast(e.message, 'error');
    }
}


