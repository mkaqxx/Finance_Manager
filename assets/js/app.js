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

function toggleTheme() {
    const currentTheme = document.documentElement.getAttribute('data-theme') || 'dark';
    const nextTheme = currentTheme === 'dark' ? 'light' : 'dark';
    changeAppTheme(nextTheme);
}

function showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;

    let icon = 'ℹ️';
    if (type === 'success') icon = '✓';
    else if (type === 'error') icon = '✕';

    toast.innerHTML = `
        <span style="font-weight: bold; font-size: 16px;">${icon}</span>
        <span style="flex: 1; line-height: 1.4;">${message}</span>
        <div class="toast-progress"></div>
    `;

    toast.addEventListener('click', () => {
        toast.classList.add('toast-hiding');
        setTimeout(() => toast.remove(), 300);
    });

    container.appendChild(toast);

    setTimeout(() => {
        if (toast.parentNode) {
            toast.classList.add('toast-hiding');
            setTimeout(() => toast.remove(), 300);
        }
    }, 3500);
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