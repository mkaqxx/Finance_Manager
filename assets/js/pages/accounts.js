//отрисовка скелетона во время загрузки данных из с++
function renderAccountsSkeleton() {
    //поиск контейнера списка счетов
    const container = document.getElementById('accounts-list');
    if (!container) return;

    //генерация временных мерцающих карточек
    let html = '';
    for (let i = 0; i < 3; i++) {
        html += `
            <div class="card" style="margin-bottom: 12px; padding: 20px;">
                <div style="display: flex; justify-content: space-between; margin-bottom: 12px;">
                    <div class="skeleton skeleton-text" style="width: 140px; height: 16px;"></div>
                    <div class="skeleton skeleton-text" style="width: 45px; height: 20px; border-radius: 10px;"></div>
                </div>
                <div style="display: flex; justify-content: space-between; align-items: flex-end;">
                    <div class="skeleton skeleton-text" style="width: 80px; height: 14px;"></div>
                    <div class="skeleton skeleton-text" style="width: 100px; height: 22px;"></div>
                </div>
            </div>
        `;
    }
    //вставка заглушек в dom
    container.innerHTML = html;
}

//загрузка списка счетов из с++ и создание карточек
async function loadAccounts() {
    try {
        //отображение скелетона перед запросом
        renderAccountsSkeleton();

        //запрос счетов из с++ бэкенда
        const accounts = await windowAPI.getAccounts();

        //получение контейнера и шаблона карточки
        const container = document.getElementById('accounts-list');
        const template = document.getElementById('tpl-account-card');
        container.innerHTML = '';

        //если счетов нет, показываем пустое состояние
        if (!accounts || !accounts.length) {
            container.innerHTML = `
                <div class="empty-state">
                    <div class="empty-state-icon">
                        <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M19 7V4a1 1 0 0 0-1-1H5a2 2 0 0 0 0 4h15a1 1 0 0 1 1 1v4h-3a2 2 0 0 0 0 4h3a1 1 0 0 0 1-1v-2a1 1 0 0 0-1-1"/><path d="M3 5v14a2 2 0 0 0 2 2h15a1 1 0 0 0 1-1v-4"/></svg>
                    </div>
                    <h4>У вас пока нет счетов</h4>
                    <p>Создайте наличный, банковский счёт или копилку для учета и контроля баланса.</p>
                    <button onclick="openAccountModal()" class="btn-primary" style="font-size: 13px;">+ Создать первый счёт</button>
                </div>
            `;
            return;
        }

        //цикл по всем полученным счетам
        accounts.forEach((acc, idx) => {
            //клонирование html-шаблона карточки
            const clone = template.content.cloneNode(true);

            //получение значка валюты
            const sym = CURRENCY_SYMBOLS[acc.currency.toLowerCase()] || acc.currency.toUpperCase();

            //иконка в зависимости от типа счета
            const iconEl = clone.querySelector('.acc-icon');
            if (iconEl) {
                iconEl.textContent = acc.type === 'bank_account' ? '💳' : acc.type === 'savings_account' ? '🎯' : '💵';
            }

            //бейдж с кодом валюты
            const badgeEl = clone.querySelector('.acc-badge');
            if (badgeEl) {
                badgeEl.textContent = acc.currency.toUpperCase();
            }

            //текстовое название типа счета
            let typeName = acc.type === 'bank_account' ? 'Банковский счёт' :
                acc.type === 'savings_account' ? 'Копилка' : 'Наличные';

            //заполнение названия, типа и баланса
            clone.querySelector('.acc-name').textContent = acc.name;
            clone.querySelector('.acc-meta').textContent = typeName;
            clone.querySelector('.acc-balance').textContent = `${acc.balance.toLocaleString()} ${sym}`;

            //проверка выполнения цели накопления
            const isGoalDone = acc.type === 'savings_account' && acc.goal && (acc.progress >= 100);

            //дополнительная информация под названием
            const numEl = clone.querySelector('.acc-number');
            if (numEl) {
                if (acc.type === 'bank_account' && acc.number) {
                    //для карты выводим маску последних 4 цифр
                    numEl.textContent = `•••• ${acc.number}`;
                    numEl.style.display = 'block';
                } else if (acc.type === 'savings_account' && acc.goal) {
                    //для копилки выводим цель и процент
                    numEl.innerHTML = `Цель: ${acc.goal.toLocaleString()} ${sym} (${(acc.progress || 0).toFixed(0)}%) ${isGoalDone ? '<span class="goal-achieved-badge">🎉 Достигнуто!</span>' : ''}`;
                    numEl.style.display = 'block';
                } else {
                    //для наличных скрываем строку
                    numEl.style.display = 'none';
                }
            }

            //настройка шкалы прогресса для копилки
            if (acc.type === 'savings_account' && acc.goal) {
                const goalWrap = clone.querySelector('.acc-goal-bar-wrap');
                const goalBar = clone.querySelector('.acc-goal-bar');
                if (goalWrap && goalBar) {
                    goalWrap.style.display = 'block';
                    //ограничение ширины шкалы от 0 до 100%
                    goalBar.style.width = `${Math.min(Math.max(acc.progress || 0, 0), 100)}%`;
                    //подсветка при достижении цели
                    if (isGoalDone) {
                        goalBar.style.backgroundColor = 'var(--color-income)';
                        goalBar.style.boxShadow = '0 0 10px var(--color-income)';
                    }
                }
            }

            //настройка анимации и событий карточки
            const cardNode = clone.querySelector('.account-card-item');
            if (cardNode) {
                //каскадное появление карточек
                cardNode.classList.add('animate-cascade');
                cardNode.style.setProperty('--item-idx', idx);

                //клик по карточке открывает окно деталей
                cardNode.addEventListener('click', () => openAccountDetailModal(acc));

                //подсветка при наведении мыши
                cardNode.addEventListener('mouseenter', () => cardNode.style.background = 'var(--bg-hover)');
                cardNode.addEventListener('mouseleave', () => cardNode.style.background = 'var(--bg-card)');
            }

            //добавление готовой карточки в список
            container.appendChild(clone);
        });
    } catch (error) {
        console.error("Ошибка загрузки счетов:", error);
    }
}
