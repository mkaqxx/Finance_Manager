let categoriesMap = {};

async function loadCategories() {
    try {
        const data = await windowAPI.getCategories();
        categoriesMap = {};
        data.forEach(cat => { categoriesMap[cat.id] = { name: cat.name, color: cat.color }; });
    } catch (error) { console.error(error); }
}

const pageTitles = {
    'dashboard': 'Главная',
    'accounts': 'Мои счета',
    'transactions': 'История транзакций',
    'budget': 'Управление бюджетом',
    'analytics': 'Аналитика'
};

function switchPage(page, element) {
    document.getElementById('page-title').innerText = pageTitles[page];
    document.querySelectorAll('.nav-links a').forEach(el => el.classList.remove('active'));
    if (element) element.classList.add('active');

    document.querySelectorAll('.page-view').forEach(el => el.style.display = 'none');
    document.getElementById(`view-${page}`).style.display = 'block';

    if (page === 'dashboard') loadDashboard();
    else if (page === 'accounts') loadAccounts();
    else if (page === 'transactions') loadTransactions();
    else if (page === 'budget') { renderCategories(); renderBudgets(); }
    else if (page === 'analytics') loadAnalytics();
}

window.addEventListener('DOMContentLoaded', async () => {
    await loadCategories();
    switchPage('dashboard');
});