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

            let extraInfo = acc.type === 'bank_account' && acc.number ? `•••• ${acc.number}` :
                acc.type === 'savings_account' && acc.goal ? `Цель: ${acc.goal.toLocaleString()} ${sym} (${acc.progress.toFixed(1)}%)` :
                    'Наличные';

            clone.querySelector('.acc-name').textContent = acc.name;
            clone.querySelector('.acc-meta').textContent = `${acc.currency.toUpperCase()} • ${extraInfo}`;
            clone.querySelector('.acc-balance').textContent = `${acc.balance.toLocaleString()} ${sym}`;

            container.appendChild(clone);
        });
    } catch (error) { console.error("Ошибка загрузки счетов:", error); }
}