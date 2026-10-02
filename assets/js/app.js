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

function closeModalAnimated(modalOrId) {
    const modal = typeof modalOrId === 'string' ? document.getElementById(modalOrId) : modalOrId;
    if (!modal || modal.style.display === 'none') return;
    modal.classList.add('modal-closing');
    setTimeout(() => {
        modal.style.display = 'none';
        modal.classList.remove('modal-closing');
    }, 180);
}

function closeAllModals() {
    const modalOverlays = document.querySelectorAll('.modal-overlay');
    let closedAny = false;
    modalOverlays.forEach(m => {
        if (m.style.display === 'flex' || m.style.display === 'block') {
            closeModalAnimated(m);
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

async function bootstrapApp() {
    await loadCategories();
    switchPage('dashboard');
}

window.addEventListener('DOMContentLoaded', () => {
    const savedTheme = localStorage.getItem('finance-app-theme') || 'dark';
    changeAppTheme(savedTheme);

    if (window.wasmApiReady) {
        bootstrapApp();
    } else {
        window.addEventListener('finance-wasm-ready', bootstrapApp, { once: true });
    }
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

// Анимация плавного накручивания чисел (Rolling Numbers / Odometer)
function animateValue(elem, start, end, duration = 750, formatter = null) {
    if (!elem) return;
    const startVal = Number(start) || 0;
    const endVal = Number(end) || 0;
    if (startVal === endVal) {
        elem.textContent = formatter ? formatter(endVal) : Math.round(endVal).toLocaleString();
        return;
    }
    const startTime = performance.now();
    const diff = endVal - startVal;

    function step(currentTime) {
        const elapsed = currentTime - startTime;
        const progress = Math.min(elapsed / duration, 1);
        // Easing: easeOutCubic
        const ease = 1 - Math.pow(1 - progress, 3);
        const current = startVal + diff * ease;

        elem.textContent = formatter ? formatter(current) : Math.round(current).toLocaleString();

        if (progress < 1) {
            requestAnimationFrame(step);
        } else {
            elem.textContent = formatter ? formatter(endVal) : Math.round(endVal).toLocaleString();
        }
    }

    requestAnimationFrame(step);
}

// Эффект микро-конфетти / салюта на Canvas
let confettiAnimId = null;
function triggerCelebrationConfetti(x = window.innerWidth / 2, y = window.innerHeight * 0.4) {
    const canvas = document.getElementById('confetti-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    canvas.style.display = 'block';

    if (confettiAnimId) cancelAnimationFrame(confettiAnimId);

    const colors = ['#00ff66', '#00e5ff', '#ff0055', '#ffd700', '#ffffff', '#72968b'];
    const particles = [];
    const count = 65;

    for (let i = 0; i < count; i++) {
        const angle = Math.random() * Math.PI * 2;
        const speed = 4 + Math.random() * 8;
        particles.push({
            x: x,
            y: y,
            vx: Math.cos(angle) * speed,
            vy: Math.sin(angle) * speed - 3,
            size: 4 + Math.random() * 6,
            color: colors[Math.floor(Math.random() * colors.length)],
            rotation: Math.random() * 360,
            rotationSpeed: (Math.random() - 0.5) * 12,
            opacity: 1,
            gravity: 0.18,
            decay: 0.012 + Math.random() * 0.012
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

// Интерактивный неоновый следящий блик на карточках
document.addEventListener('mousemove', (e) => {
    const card = e.target.closest('.card, .account-card-item');
    if (card) {
        const rect = card.getBoundingClientRect();
        card.style.setProperty('--mouse-x', `${e.clientX - rect.left}px`);
        card.style.setProperty('--mouse-y', `${e.clientY - rect.top}px`);
    }
});

// Неоновый Ripple-эффект при нажатии на кнопки
document.addEventListener('click', (e) => {
    const btn = e.target.closest('.btn-primary, .btn-secondary');
    if (!btn) return;
    const rect = btn.getBoundingClientRect();
    const circle = document.createElement('span');
    const diameter = Math.max(rect.width, rect.height);
    const radius = diameter / 2;
    circle.style.width = circle.style.height = `${diameter}px`;
    circle.style.left = `${e.clientX - rect.left - radius}px`;
    circle.style.top = `${e.clientY - rect.top - radius}px`;
    circle.classList.add('btn-ripple');
    const oldRipple = btn.querySelector('.btn-ripple');
    if (oldRipple) oldRipple.remove();
    btn.appendChild(circle);
    setTimeout(() => circle.remove(), 600);
});