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
    
    if (!element) {
        element = document.querySelector(`.nav-links a[onclick*="'${page}'"]`);
    }
    if (element) element.classList.add('active');

    document.querySelectorAll('.page-view').forEach(el => el.style.display = 'none');
    document.getElementById(`view-${page}`).style.display = 'block';

    if (page === 'dashboard') loadDashboard();
    else if (page === 'accounts') loadAccounts();
    else if (page === 'transactions') loadTransactions();
    else if (page === 'budget') { renderCategories(); renderBudgets(); }
    else if (page === 'analytics') loadAnalytics();
}

function closeAllModals() {
    const modalOverlays = document.querySelectorAll('.modal-overlay');
    let closedAny = false;
    modalOverlays.forEach(m => {
        if (m.style.display === 'flex' || m.style.display === 'block') {
            m.style.display = 'none';
            closedAny = true;
        }
    });
    return closedAny;
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

    window.dispatchEvent(new CustomEvent('themeChanged', { detail: { theme: validTheme } }));
}

window.addEventListener('DOMContentLoaded', async () => {
    const savedTheme = localStorage.getItem('finance-app-theme') || 'dark';
    changeAppTheme(savedTheme);

    await loadCategories();
    switchPage('dashboard');
});

// Глобальные горячие клавиши (Hotkeys)
window.addEventListener('keydown', (e) => {
    const activeEl = document.activeElement;
    const isInput = activeEl && (
        activeEl.tagName === 'INPUT' ||
        activeEl.tagName === 'TEXTAREA' ||
        activeEl.tagName === 'SELECT' ||
        activeEl.isContentEditable
    );

    // Escape: всегда закрывает открытые модальные окна
    if (e.key === 'Escape') {
        if (isInput) activeEl.blur();
        const closed = closeAllModals();
        if (closed) {
            e.preventDefault();
            return;
        }
    }

    // Если фокус в поле ввода — не перехватываем другие горячие клавиши
    if (isInput) return;

    // N или Ctrl+N: создание новой транзакции
    if ((e.ctrlKey && e.code === 'KeyN') || (!e.ctrlKey && !e.metaKey && !e.altKey && e.code === 'KeyN')) {
        e.preventDefault();
        if (typeof openTxModal === 'function') {
            openTxModal();
        }
        return;
    }

    // Цифры 1..5: Быстрая навигация по вкладкам
    if (!e.ctrlKey && !e.metaKey && !e.altKey) {
        const pageMap = {
            'Digit1': 'dashboard',
            'Digit2': 'accounts',
            'Digit3': 'transactions',
            'Digit4': 'budget',
            'Digit5': 'analytics',
            'Numpad1': 'dashboard',
            'Numpad2': 'accounts',
            'Numpad3': 'transactions',
            'Numpad4': 'budget',
            'Numpad5': 'analytics'
        };
        if (pageMap[e.code]) {
            e.preventDefault();
            switchPage(pageMap[e.code]);
        }
    }
});