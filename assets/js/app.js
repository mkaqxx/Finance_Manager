//словарь категорий по айди: { id: { name, color } }
let categoriesMap = {};

//загрузка категорий из с++ и заполнение словаря
async function loadCategories() {
    try {
        const data = await windowAPI.getCategories();
        categoriesMap = {};
        data.forEach(cat => { categoriesMap[cat.id] = { name: cat.name, color: cat.color }; });
    } catch (error) { 
        console.error("Ошибка загрузки категорий:", error); 
    }
}

//заголовки страниц для шапки
const pageTitles = {
    'dashboard': 'Главная панель',
    'accounts': 'Мои счета',
    'transactions': 'История транзакций',
    'budget': 'Управление бюджетом',
    'analytics': 'Финансовая аналитика'
};

//переключение страниц
function switchPage(page, element) {
    const titleEl = document.getElementById('page-title');
    if (titleEl) titleEl.innerText = pageTitles[page] || 'Главная';

    document.querySelectorAll('.nav-links a').forEach(el => el.classList.remove('active'));
        
    if (!element) {
        element = document.querySelector(`.nav-links a[onclick*="'${page}'"]`);
    }
    if (element) element.classList.add('active');

    //скрытие всех страниц
    document.querySelectorAll('.page-view').forEach(el => el.style.display = 'none');
    
    //показ выбранной страницы
    const targetPage = document.getElementById(`view-${page}`);
    if (targetPage) targetPage.style.display = 'block';

    //загрузка данных для выбранной страницы
    if (page === 'dashboard') loadDashboard();
    else if (page === 'accounts') loadAccounts();
    else if (page === 'transactions') loadTransactions();
    else if (page === 'budget') { renderCategories(); renderBudgets(); }
    else if (page === 'analytics') {
        requestAnimationFrame(() => loadAnalytics());
    }
}

// ========================================================
// АНИМАЦИИ И УТИЛИТЫ ФОРМАТИРОВАНИЯ ЧИСЕЛ (COUNTUP)
// ========================================================

// Форматирование чисел: пробелы между тысячами и 2 знака после запятой
function formatMoneyWithSpaces(val, decimals = 2) {
    const num = Number(val) || 0;
    const fixed = num.toFixed(decimals);
    const parts = fixed.split('.');
    parts[0] = parts[0].replace(/\B(?=(\d{3})+(?!\d))/g, ' ');
    return parts.join('.');
}

// Форматирование суммы с разделением целой части и копеек в отдельные span для изящной типографики
function formatMoneyPartsHtml(amount, decimals = 2, suffix = '') {
    const num = Number(amount) || 0;
    const fixed = num.toFixed(decimals);
    const parts = fixed.split('.');
    const intPart = parts[0].replace(/\B(?=(\d{3})+(?!\d))/g, ' ');
    const decPart = parts.length > 1 ? '.' + parts[1] : '';
    const suffHtml = suffix ? `<span class="amount-currency">${suffix}</span>` : '';
    return `<span class="amount-int">${intPart}</span><span class="amount-dec">${decPart}</span>${suffHtml}`;
}

// Плавная анимация набегания чисел (CountUp) за ~800мс с кубическим ease-out
function animateValue(element, startVal, endVal, duration = 800, formatter = null, isHtml = false) {
    if (!element) return;
    const start = Number(startVal) || 0;
    const end = Number(endVal) || 0;
    const startTime = performance.now();

    function easeOutCubic(t) {
        return 1 - Math.pow(1 - t, 3);
    }

    function step(currentTime) {
        const elapsed = currentTime - startTime;
        const progress = Math.min(elapsed / duration, 1);
        const currentVal = start + (end - start) * easeOutCubic(progress);

        const res = typeof formatter === 'function' ? formatter(currentVal) : formatMoneyWithSpaces(currentVal, 2);
        if (isHtml) {
            element.innerHTML = res;
        } else {
            element.textContent = res;
        }

        if (progress < 1) {
            requestAnimationFrame(step);
        } else {
            const finalRes = typeof formatter === 'function' ? formatter(end) : formatMoneyWithSpaces(end, 2);
            if (isHtml) {
                element.innerHTML = finalRes;
            } else {
                element.textContent = finalRes;
            }
        }
    }

    requestAnimationFrame(step);
}

// Удобная обертка для анимации сумм с суффиксом валюты
function animateCountUp(element, endVal, duration = 800, suffix = '', decimals = 2, splitDecimals = false) {
    if (!element) return;
    const isSplit = splitDecimals || (element.classList && element.classList.contains('split-amount'));
    animateValue(element, 0, endVal, duration, (val) => {
        if (isSplit) {
            return formatMoneyPartsHtml(val, decimals, suffix);
        }
        const formatted = formatMoneyWithSpaces(val, decimals);
        return suffix ? `${formatted} ${suffix}` : formatted;
    }, isSplit);
}

//плавное закрытие модального окна с анимацией
function closeModalAnimated(modalOrId) {
    const modal = typeof modalOrId === 'string' ? document.getElementById(modalOrId) : modalOrId;
    if (!modal) return;
    
    // Проверка для Sheet/Drawer
    if (modal.classList.contains('sheet-overlay')) {
        closeAccountDetailModal();
        return;
    }

    if (modal.style.display === 'none') return;
    modal.classList.add('modal-closing');
    setTimeout(() => {
        modal.style.display = 'none';
        modal.classList.remove('modal-closing');
    }, 150);
}

//закрытие всех открытых окон, модалок и боковых панелей
function closeAllModals() {
    let closedAny = false;

    // 1. Проверяем Sheet / Drawer
    const sheet = document.getElementById('account-detail-modal');
    if (sheet && sheet.classList.contains('sheet-open')) {
        closeAccountDetailModal();
        closedAny = true;
    }

    // 2. Проверяем Command Palette
    const cmdModal = document.getElementById('command-palette-modal');
    if (cmdModal && cmdModal.style.display === 'flex') {
        closeCommandPalette();
        closedAny = true;
    }

    // 3. Проверяем все остальные модалки
    const modalOverlays = document.querySelectorAll('.modal-overlay');
    modalOverlays.forEach(m => {
        if (m.style.display === 'flex' || m.style.display === 'block') {
            closeModalAnimated(m);
            closedAny = true;
        }
    });

    return closedAny;
}

//переключение темы оформления (светлая/темная)
function toggleTheme() {
    const currentTheme = document.documentElement.getAttribute('data-theme') || 'dark';
    const nextTheme = currentTheme === 'dark' ? 'light' : 'dark';
    changeAppTheme(nextTheme);
}

//установка темы оформления и сохранение выбора
function changeAppTheme(themeName) {
    const validTheme = (themeName === 'light') ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', validTheme);
    localStorage.setItem('finance-app-theme', validTheme);

    // Уведомление для Chart.js
    window.dispatchEvent(new CustomEvent('themeChanged', { detail: { theme: validTheme } }));
}

//всплывающее тост-уведомление
function showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;

    let icon = 'ℹ️';
    if (type === 'success') icon = '✓';
    else if (type === 'error') icon = '✕';

    toast.innerHTML = `
        <span style="font-weight: bold; font-size: 15px; color: ${type === 'success' ? 'var(--color-income)' : type === 'error' ? 'var(--color-expense)' : 'var(--text-muted)'};">${icon}</span>
        <span class="toast-text" style="flex: 1; line-height: 1.4;"></span>
        <div class="toast-progress"></div>
    `;
    toast.querySelector('.toast-text').textContent = message;

    toast.addEventListener('click', () => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateY(10px)';
        setTimeout(() => toast.remove(), 200);
    });

    container.appendChild(toast);

    setTimeout(() => {
        if (toast.parentNode) {
            toast.style.opacity = '0';
            toast.style.transform = 'translateY(10px)';
            setTimeout(() => toast.remove(), 250);
        }
    }, 3500);
}

//анимация конфетти при успехе
let confettiAnimId = null;
function triggerCelebrationConfetti(x = window.innerWidth / 2, y = window.innerHeight * 0.4) {
    const canvas = document.getElementById('confetti-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    canvas.style.display = 'block';

    if (confettiAnimId) cancelAnimationFrame(confettiAnimId);

    const colors = ['#10b981', '#38bdf8', '#f59e0b', '#f43f5e', '#6366f1', '#ffffff'];
    const particles = [];
    const count = 55;

    for (let i = 0; i < count; i++) {
        const angle = Math.random() * Math.PI * 2;
        const speed = 4 + Math.random() * 7;
        particles.push({
            x: x,
            y: y,
            vx: Math.cos(angle) * speed,
            vy: Math.sin(angle) * speed - 3,
            size: 4 + Math.random() * 5,
            color: colors[Math.floor(Math.random() * colors.length)],
            rotation: Math.random() * 360,
            rotationSpeed: (Math.random() - 0.5) * 10,
            opacity: 1,
            gravity: 0.16,
            decay: 0.014 + Math.random() * 0.01
        });
    }

    function renderConfetti() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        let active = false;

        particles.forEach(p => {
            p.x += p.vx;
            p.y += p.vy;
            p.vy += p.gravity;
            p.vx *= 0.98;
            p.rotation += p.rotationSpeed;
            p.opacity -= p.decay;

            if (p.opacity > 0) {
                active = true;
                ctx.save();
                ctx.translate(p.x, p.y);
                ctx.rotate((p.rotation * Math.PI) / 180);
                ctx.globalAlpha = Math.max(0, p.opacity);
                ctx.fillStyle = p.color;
                ctx.fillRect(-p.size / 2, -p.size / 2, p.size, p.size * 0.7);
                ctx.restore();
            }
        });

        if (active) {
            confettiAnimId = requestAnimationFrame(renderConfetti);
        } else {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            canvas.style.display = 'none';
            confettiAnimId = null;
        }
    }

    renderConfetti();
}

//инициализация приложения при загрузке DOM
window.addEventListener('DOMContentLoaded', async () => {
    const savedTheme = localStorage.getItem('finance-app-theme') || 'dark';
    changeAppTheme(savedTheme);

    await loadCategories();
    switchPage('dashboard');
});

//глобальные горячие клавиши (Ctrl+K, Esc, N, 1..5)
window.addEventListener('keydown', (e) => {
    // Ctrl+K или Cmd+K: Command Palette
    if ((e.ctrlKey || e.metaKey) && (e.code === 'KeyK' || e.key === 'k' || e.key === 'K' || e.key === 'л' || e.key === 'Л')) {
        e.preventDefault();
        openCommandPalette();
        return;
    }

    const activeEl = document.activeElement;
    const isInput = activeEl && (
        activeEl.tagName === 'INPUT' ||
        activeEl.tagName === 'TEXTAREA' ||
        activeEl.tagName === 'SELECT' ||
        activeEl.isContentEditable
    );

    // Escape: закрытие открытых окон
    if (e.key === 'Escape') {
        if (isInput) activeEl.blur();
        const closed = closeAllModals();
        if (closed) {
            e.preventDefault();
            return;
        }
    }

    if (isInput) return;

    // N: новая транзакция
    if ((e.ctrlKey && e.code === 'KeyN') || (!e.ctrlKey && !e.metaKey && !e.altKey && (e.code === 'KeyN' || e.key === 'n' || e.key === 'т'))) {
        e.preventDefault();
        if (typeof openTxModal === 'function') {
            openTxModal();
        }
        return;
    }

    // 1..5: навигация по разделам
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