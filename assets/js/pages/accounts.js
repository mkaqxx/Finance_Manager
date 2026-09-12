async function loadAccounts() {
    try {
        const accounts = await windowAPI.getAccounts();
        const container = document.getElementById('accounts-list');
        const template = document.getElementById('tpl-account-card');
        container.innerHTML = '';

        const currencySymbols = { 'rub': '₽', 'usd': '$', 'eur': '€', 'byn': 'Br' };

        accounts.forEach(acc => {
            const clone = template.content.cloneNode(true);
            const sym = currencySymbols[acc.currency.toLowerCase()] || acc.currency.toUpperCase();

            let extraInfo = '';
            if (acc.type === 'bank_account' && acc.number) {
                extraInfo = `•••• ${acc.number}`;
            } else if (acc.type === 'savings_account') {
                extraInfo = `
                <div>Цель: ${acc.goal.toLocaleString()} ${sym}</div>
                <div>Прогресс: ${acc.progress.toFixed(1)}%</div>
                <div>Нужно откладывать: ${acc.monthly_required.toLocaleString()} ${sym}/мес</div>
                <div>Дедлайн: ${acc.deadline}</div>
                `;
            } else {
                extraInfo = 'Наличные';
            }

            clone.querySelector('.acc-name').textContent = acc.name;
            clone.querySelector('.acc-meta').innerHTML = `${acc.currency.toUpperCase()} • ${extraInfo}`;
            clone.querySelector('.acc-balance').textContent = `${acc.balance.toLocaleString()} ${sym}`;

            // Данные для скрытого блока
            clone.querySelector('.det-id').textContent = acc.id;
            clone.querySelector('.det-type').textContent = acc.type;

            // Логика разворачивания (accordion)
            const cardNode = clone.querySelector('.account-card-item');
            const detailsNode = clone.querySelector('.acc-details');

            cardNode.addEventListener('click', () => {
                const isHidden = detailsNode.style.display === 'none';
                document.querySelectorAll('.acc-details').forEach(el => el.style.display = 'none');
                if (isHidden) detailsNode.style.display = 'block';
            });

            // Эффекты наведения
            cardNode.addEventListener('mouseenter', () => cardNode.style.background = '#222222');
            cardNode.addEventListener('mouseleave', () => cardNode.style.background = '#181818');

            container.appendChild(clone);
        });
    } catch (error) { console.error("Ошибка загрузки счетов:", error); }
}