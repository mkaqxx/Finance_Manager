async function loadTransactions() {
    try {
        const data = await windowAPI.getTransactions();
        const tbody = document.getElementById('transactions-table');
        const template = document.getElementById('tpl-transaction-row');
        tbody.innerHTML = '';

        data.forEach(tx => {
            const clone = template.content.cloneNode(true);
            const category = categoriesMap[tx.category_id] || { name: "Неизвестно", color: "#ffffff" };
            const sign = tx.type === "income" ? '+' : '-';

            clone.querySelector('.tx-date').textContent = tx.date;
            const catCell = clone.querySelector('.tx-category');
            catCell.textContent = category.name;
            catCell.style.color = category.color;
            clone.querySelector('.tx-amount').textContent = `${sign}${tx.amount.toLocaleString()} Br`;

            tbody.appendChild(clone);
        });
    } catch (error) { console.error("Ошибка загрузки транзакций:", error); }
}