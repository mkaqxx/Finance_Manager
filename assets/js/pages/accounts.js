async function loadAccounts() {
    try {
        const accounts = await windowAPI.getAccounts();
        const container = document.getElementById('accounts-list');
        const template = document.getElementById('tpl-account-card');
        container.innerHTML = '';

        accounts.forEach(acc => {
            const clone = template.content.cloneNode(true);
            const sym = CURRENCY_SYMBOLS[acc.currency.toLowerCase()] || acc.currency.toUpperCase();

            const iconEl = clone.querySelector('.acc-icon');
            if (iconEl) {
                iconEl.textContent = acc.type === 'bank_account' ? '💳' : acc.type === 'savings_account' ? '🎯' : '💵';
            }

            const badgeEl = clone.querySelector('.acc-badge');
            if (badgeEl) {
                badgeEl.textContent = acc.currency.toUpperCase();
            }

            let typeName = acc.type === 'bank_account' ? 'Банковский счёт' :
                acc.type === 'savings_account' ? 'Копилка' : 'Наличные';

            clone.querySelector('.acc-name').textContent = acc.name;
            clone.querySelector('.acc-meta').textContent = typeName;
            clone.querySelector('.acc-balance').textContent = `${acc.balance.toLocaleString()} ${sym}`;

            const numEl = clone.querySelector('.acc-number');
            if (numEl) {
                if (acc.type === 'bank_account' && acc.number) {
                    numEl.textContent = `•••• ${acc.number}`;
                    numEl.style.display = 'block';
                } else if (acc.type === 'savings_account' && acc.goal) {
                    numEl.textContent = `Цель: ${acc.goal.toLocaleString()} ${sym} (${(acc.progress || 0).toFixed(0)}%)`;
                    numEl.style.display = 'block';
                } else {
                    numEl.style.display = 'none';
                }
            }

            if (acc.type === 'savings_account' && acc.goal) {
                const goalWrap = clone.querySelector('.acc-goal-bar-wrap');
                const goalBar = clone.querySelector('.acc-goal-bar');
                if (goalWrap && goalBar) {
                    goalWrap.style.display = 'block';
                    goalBar.style.width = `${Math.min(Math.max(acc.progress || 0, 0), 100)}%`;
                }
            }

            const cardNode = clone.querySelector('.account-card-item');

            // При клике открываем модалку и передаем туда ID счета
            cardNode.addEventListener('click', () => openAccountDetailModal(acc));

            // Эффекты наведения
            cardNode.addEventListener('mouseenter', () => cardNode.style.background = 'var(--bg-hover)');
            cardNode.addEventListener('mouseleave', () => cardNode.style.background = 'var(--bg-card)');

            container.appendChild(clone);
        });
    } catch (error) { console.error("Ошибка загрузки счетов:", error); }
}
