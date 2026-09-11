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
            clone.querySelector('.cat-delete-btn').onclick = () => alert(`Удаление ID: ${cat.id}`);
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

            container.appendChild(clone);
        });
    } catch (e) { console.error(e); }
}