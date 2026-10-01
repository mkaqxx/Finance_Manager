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

function changeAppTheme(themeName) {
    const validTheme = (themeName === 'light') ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', validTheme);
    localStorage.setItem('finance-app-theme', validTheme);

    const selector = document.getElementById('theme-selector');
    if (selector && selector.value !== validTheme) {
        selector.value = validTheme;
    }

    window.dispatchEvent(new CustomEvent('themeChanged', { detail: { theme: validTheme } }));
}

window.addEventListener('DOMContentLoaded', async () => {
    const savedTheme = localStorage.getItem('finance-app-theme') || 'dark';
    changeAppTheme(savedTheme);

    const selector = document.getElementById('theme-selector');
    if (selector) {
        selector.value = savedTheme;
        selector.addEventListener('change', (e) => changeAppTheme(e.target.value));
    }

    await loadCategories();
    switchPage('dashboard');
});