//словарь категорий по айди: { id: { name, color } }
let categoriesMap = {};

//загрузка категорий из с++ и заполнение словаря
async function loadCategories() {
    try {
        //вызов метода getCategories через api
        const data = await windowAPI.getCategories();
        //очистка словаря
        categoriesMap = {};
        //заполнение хэш-таблицы категорий
        data.forEach(cat => { categoriesMap[cat.id] = { name: cat.name, color: cat.color }; });
    } catch (error) { 
        console.error("Ошибка загрузки категорий:", error); 
    }
}

//заголовки страниц для шапки
const pageTitles = {
    'dashboard': 'Главная',
    'accounts': 'Мои счета',
    'transactions': 'История транзакций',
    'budget': 'Управление бюджетом',
    'analytics': 'Аналитика'
};

//переключение страниц
function switchPage(page, element) {
    //установка заголовка страницы
    document.getElementById('page-title').innerText = pageTitles[page];
    //снятие активного класса со ссылок навигации
    document.querySelectorAll('.nav-links a').forEach(el => el.classList.remove('active'));
        
    //поиск элемента навигации если не передан явно
    if (!element) {
        element = document.querySelector(`.nav-links a[onclick*="'${page}'"]`);
    }
    //подсветка активной ссылки
    if (element) element.classList.add('active');

    //скрытие всех страниц
    document.querySelectorAll('.page-view').forEach(el => el.style.display = 'none');
    //показ выбранной страницы
    document.getElementById(`view-${page}`).style.display = 'block';

    //загрузка данных для выбранной страницы
    if (page === 'dashboard') loadDashboard();
    else if (page === 'accounts') loadAccounts();
    else if (page === 'transactions') loadTransactions();
    else if (page === 'budget') { renderCategories(); renderBudgets(); }
    else if (page === 'analytics') loadAnalytics();
}

//плавное закрытие модального окна с анимацией
function closeModalAnimated(modalOrId) {
    const modal = typeof modalOrId === 'string' ? document.getElementById(modalOrId) : modalOrId;
    if (!modal || modal.style.display === 'none') return;
    //добавление класса анимации закрытия
    modal.classList.add('modal-closing');
    //скрытие окна после окончания анимации (180 мс)
    setTimeout(() => {
        modal.style.display = 'none';
        modal.classList.remove('modal-closing');
    }, 180);
}

//закрытие всех открытых модальных окон
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

//переключение темы оформления (светлая/темная)
function toggleTheme() {
    const currentTheme = document.documentElement.getAttribute('data-theme') || 'dark';
    const nextTheme = currentTheme === 'dark' ? 'light' : 'dark';
    changeAppTheme(nextTheme);
}

//установка темы оформления и сохранение выбора
function changeAppTheme(themeName) {
    const validTheme = (themeName === 'light') ? 'light' : 'dark';
    //установка атрибута data-theme у тега html
    document.documentElement.setAttribute('data-theme', validTheme);
    //сохранение выбора в localstorage
    localStorage.setItem('finance-app-theme', validTheme);

    //отправка события themeChanged для обновления графиков
    window.dispatchEvent(new CustomEvent('themeChanged', { detail: { theme: validTheme } }));
}

//создание и отображение всплывающего уведомления (toast)
function showToast(message, type = 'info') {
    //поиск контейнера уведомлений
    const container = document.getElementById('toast-container');
    if (!container) return;

    //создание элемента плашки
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;

    //значок по типу уведомления
    let icon = 'ℹ️';
    if (type === 'success') icon = '✓';
    else if (type === 'error') icon = '✕';

    //разметка тоста с индикатором времени
    toast.innerHTML = `
        <span style="font-weight: bold; font-size: 16px;">${icon}</span>
        <span class="toast-text" style="flex: 1; line-height: 1.4;"></span>
        <div class="toast-progress"></div>
    `;
    toast.querySelector('.toast-text').textContent = message;

    //клик по тосту скрывает его досрочно
    toast.addEventListener('click', () => {
        toast.classList.add('toast-hiding');
        setTimeout(() => toast.remove(), 300);
    });

    //добавление тоста в контейнер
    container.appendChild(toast);

    //автоматическое удаление через 3.5 секунды
    setTimeout(() => {
        if (toast.parentNode) {
            toast.classList.add('toast-hiding');
            setTimeout(() => toast.remove(), 300);
        }
    }, 3500);
}

//инициализация приложения при загрузке dom
window.addEventListener('DOMContentLoaded', async () => {
    //восстановление сохраненной темы оформления
    const savedTheme = localStorage.getItem('finance-app-theme') || 'dark';
    changeAppTheme(savedTheme);

    //загрузка категорий из с++ бэкенда
    await loadCategories();
    //открытие главной страницы по умолчанию
    switchPage('dashboard');
});

//глобальные горячие клавиши
window.addEventListener('keydown', (e) => {
    //проверка фокуса в поле ввода
    const activeEl = document.activeElement;
    const isInput = activeEl && (
        activeEl.tagName === 'INPUT' ||
        activeEl.tagName === 'TEXTAREA' ||
        activeEl.tagName === 'SELECT' ||
        activeEl.isContentEditable
    );

    //escape: закрытие открытых модальных окон
    if (e.key === 'Escape') {
        if (isInput) activeEl.blur();
        const closed = closeAllModals();
        if (closed) {
            e.preventDefault();
            return;
        }
    }

    //если фокус в поле ввода, другие клавиши не перехватываем
    if (isInput) return;

    //n или ctrl+n: создание новой транзакции
    if ((e.ctrlKey && e.code === 'KeyN') || (!e.ctrlKey && !e.metaKey && !e.altKey && e.code === 'KeyN')) {
        e.preventDefault();
        if (typeof openTxModal === 'function') {
            openTxModal();
        }
        return;
    }

    //клавиши 1..5: навигация по вкладкам
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

//анимация нарастания чисел (rolling numbers)
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

    //шаг анимации на каждый кадр
    function step(currentTime) {
        const elapsed = currentTime - startTime;
        const progress = Math.min(elapsed / duration, 1);
        //функция плавности easeOutCubic
        const ease = 1 - Math.pow(1 - progress, 3);
        const current = startVal + diff * ease;

        elem.textContent = formatter ? formatter(current) : Math.round(current).toLocaleString();

        //запрос следующего кадра или завершение анимации
        if (progress < 1) {
            requestAnimationFrame(step);
        } else {
            elem.textContent = formatter ? formatter(endVal) : Math.round(endVal).toLocaleString();
        }
    }

    requestAnimationFrame(step);
}

//салют конфетти на canvas
let confettiAnimId = null;
function triggerCelebrationConfetti(x = window.innerWidth / 2, y = window.innerHeight * 0.4) {
    const canvas = document.getElementById('confetti-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    canvas.style.display = 'block';

    //остановка предыдущей анимации
    if (confettiAnimId) cancelAnimationFrame(confettiAnimId);

    const colors = ['#00ff66', '#00e5ff', '#ff0055', '#ffd700', '#ffffff', '#72968b'];
    const particles = [];
    const count = 65;

    //генерация массива частиц со скоростями и цветами
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

    //отрисовка физики движения частиц
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
            //очистка и скрытие холста при завершении
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            canvas.style.display = 'none';
            confettiAnimId = null;
        }
    }

    renderConfetti();
}

//интерактивный неоновый блик карточки за курсором
document.addEventListener('mousemove', (e) => {
    const card = e.target.closest('.card, .account-card-item');
    if (card) {
        const rect = card.getBoundingClientRect();
        //передача координат мыши в css-переменные карточки
        card.style.setProperty('--mouse-x', `${e.clientX - rect.left}px`);
        card.style.setProperty('--mouse-y', `${e.clientY - rect.top}px`);
    }
});

//неоновый ripple-эффект при нажатии на кнопки
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
    //удаление элемента волны после завершения анимации
    setTimeout(() => circle.remove(), 600);
});