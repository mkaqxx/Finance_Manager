async function loadTransactions() {
    try {
        const data = await windowAPI.getTransactions();
        const tbody = document.getElementById('transactions-table');
        const template = document.getElementById('tpl-transaction-row');
        tbody.innerHTML = '';

        data.forEach(tx => {
            const clone = template.content.cloneNode(true);
            const category = categoriesMap[tx.category_id] || { name: "-", color: "#ffffff" };
            const sign = tx.type === "income" ? '+' : '-';

            clone.querySelector('.tx-date').textContent = tx.date;
            const catCell = clone.querySelector('.tx-category');
            catCell.textContent = category.name;
            catCell.style.color = category.color;
            clone.querySelector('.tx-amount').textContent = `${sign}${tx.amount.toLocaleString()} Br`;
            clone.querySelector('.tx-delete-btn').onclick = () => handleRemoveTransaction(tx.id);

            tbody.appendChild(clone);
        });
    } catch (error) { console.error("Ошибка загрузки транзакций:", error); }
}


async function handleRemoveTransaction(id){
    try{
        await windowAPI.removeTransaction(id);
        loadTransactions();
    }
    catch (e){
        console.error("Ошибка удаления: " + e.message);
    }
}