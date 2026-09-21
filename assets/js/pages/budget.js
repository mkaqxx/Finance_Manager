async function renderCategories() {
    try {
        const categories = await windowAPI.getCategories();
        const container = document.getElementById('categories-list');
        const template = document.getElementById('tpl-category-card');
        container.innerHTML = categories.length ? '' : '<span style="color: #b3b3b3;">Нет категорий.</span>';

        categories.forEach(cat => {
            const clone = template.content.cloneNode(true);
            clone.querySelector('.cat-color-dot').style.backgroundColor = cat.color;
            clone.querySelector('.cat-name').textContent = cat.name;
            clone.querySelector('.cat-type').textContent = cat.type === 'expense' ? 'Расход' : 'Доход';

            // Вызов функции удаления категории
            clone.querySelector('.cat-delete-btn').onclick = () => handleRemoveCategory(cat.id);

            container.appendChild(clone);
        });
    } catch (e) { console.error(e); }
}

async function renderBudgets() {
    try {
        const budgets = await windowAPI.getBudgets();
        const container = document.getElementById('budget-list');
        const template = document.getElementById('tpl-budget-bar');
        container.innerHTML = budgets.length ? '' : '<span style="color: #b3b3b3;">Лимиты не установлены.</span>';

        budgets.forEach(b => {
            const clone = template.content.cloneNode(true);
            const percent = Math.min((b.current_amount / b.limit) * 100, 100).toFixed(1);
            let barColor = b.status === 'exceeded' ? '#e91429' : b.status === 'warning' ? '#f39c12' : '#1db954';

            const nameEl = clone.querySelector('.b-name');
            nameEl.textContent = b.name;
            nameEl.style.color = b.color;
            clone.querySelector('.b-current').textContent = b.current_amount.toLocaleString();
            clone.querySelector('.b-limit').textContent = `${b.limit.toLocaleString()} Br`;
            clone.querySelector('.b-percent-text').textContent = percent;

            const bar = clone.querySelector('.b-progress-bar');
            bar.style.width = `${percent}%`;
            bar.style.backgroundColor = barColor;

            // ИСПРАВЛЕНО: передаем b.category_id вместо b.id
            clone.querySelector('.bud-delete-btn').onclick = () => handleRemoveBudget(b.category_id);

            container.appendChild(clone);
        });
    } catch (e) { console.error(e); }
}

// === ФУНКЦИИ УДАЛЕНИЯ (обязательно должны быть здесь) ===

async function handleRemoveCategory(id) {
    try {
        await windowAPI.removeCategory(id);
        await loadCategories();
        renderCategories();
        renderBudgets();
    } catch (e) {
        // ТЕПЕРЬ ОШИБКА БУДЕТ ВИДНА
        console.error("Ошибка удаления: " + e.message);
    }
}

async function handleRemoveBudget(categoryId) {
    try {
        await windowAPI.removeBudget(categoryId);
        renderBudgets();
    } catch (e) {
        // ТЕПЕРЬ ОШИБКА БУДЕТ ВИДНА
        console.error("Ошибка удаления: " + e.message);
    }
}