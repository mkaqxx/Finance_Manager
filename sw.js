//имя кэша для версионирования статических ресурсов
const CACHE_NAME = 'finance-manager-v4';

//список всех файлов для оффлайн работы приложения
const STATIC_ASSETS = [
    './',
    './index.html',
    './manifest.json',
    './assets/icons/icon-192.png',
    './assets/icons/icon-256.png',
    './assets/icons/icon-512.png',
    './assets/wasm/finance_manager.js',
    './assets/wasm/finance_manager.wasm',
    './assets/js/chart.js',
    './assets/js/api.js',
    './assets/js/app.js',
    './assets/js/modals.js',
    './assets/js/wasm-loader.js',
    './assets/js/pages/accounts.js',
    './assets/js/pages/analytics.js',
    './assets/js/pages/budget.js',
    './assets/js/pages/dashboard.js',
    './assets/js/pages/transactions.js'
];

//установка service worker и кэширование статики
self.addEventListener('install', (event) => {
    event.waitUntil(
        caches.open(CACHE_NAME).then((cache) => {
            console.log('[SW] Кэширование статических файлов приложения');
            return cache.addAll(STATIC_ASSETS);
        }).then(() => self.skipWaiting())
    );
});

//активация service worker и удаление старых версий кэша
self.addEventListener('activate', (event) => {
    event.waitUntil(
        caches.keys().then((keys) => {
            return Promise.all(
                keys.map((key) => {
                    //удаление старого кэша если имя не совпадает
                    if (key !== CACHE_NAME) {
                        console.log('[SW] Удаление устаревшего кэша:', key);
                        return caches.delete(key);
                    }
                })
            );
        }).then(() => self.clients.claim())
    );
});

//перехват сетевых запросов (cache-first стратегия для оффлайн работы)
self.addEventListener('fetch', (event) => {
    //пропускаем запросы отличные от GET
    if (event.request.method !== 'GET') return;

    event.respondWith(
        caches.match(event.request).then((cachedResponse) => {
            //если файл уже есть в кэше, отдаем его без сети
            if (cachedResponse) {
                return cachedResponse;
            }
            //иначе загружаем из сети и сохраняем копию в кэш
            return fetch(event.request).then((networkResponse) => {
                //проверка валидности ответа
                if (!networkResponse || networkResponse.status !== 200 || networkResponse.type !== 'basic') {
                    return networkResponse;
                }
                const responseToCache = networkResponse.clone();
                caches.open(CACHE_NAME).then((cache) => {
                    cache.put(event.request, responseToCache);
                });
                return networkResponse;
            }).catch(() => {
                //оффлайн фоллбэк если нет сети
                if (event.request.mode === 'navigate') {
                    return caches.match('./index.html');
                }
            });
        })
    );
});
