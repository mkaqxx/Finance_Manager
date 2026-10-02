const CACHE_NAME = 'finance-manager-v3';

const STATIC_ASSETS = [
    './',
    './index.html',
    './manifest.json',
    './sw.js',
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

self.addEventListener('install', (event) => {
    event.waitUntil(
        caches.open(CACHE_NAME).then((cache) => {
            console.log('[SW] Pre-caching static assets');
            return cache.addAll(STATIC_ASSETS);
        }).then(() => self.skipWaiting())
    );
});

self.addEventListener('activate', (event) => {
    event.waitUntil(
        caches.keys().then((keys) => {
            return Promise.all(
                keys.map((key) => {
                    if (key !== CACHE_NAME) {
                        console.log('[SW] Removing old cache', key);
                        return caches.delete(key);
                    }
                })
            );
        }).then(() => self.clients.claim())
    );
});

self.addEventListener('fetch', (event) => {
    // Не кэшируем запросы, не являющиеся GET
    if (event.request.method !== 'GET') return;

    event.respondWith(
        caches.match(event.request).then((cachedResponse) => {
            if (cachedResponse) {
                return cachedResponse;
            }
            return fetch(event.request).then((networkResponse) => {
                // Кэшируем только валидные ответы
                if (!networkResponse || networkResponse.status !== 200 || networkResponse.type !== 'basic') {
                    return networkResponse;
                }
                const responseToCache = networkResponse.clone();
                caches.open(CACHE_NAME).then((cache) => {
                    cache.put(event.request, responseToCache);
                });
                return networkResponse;
            });
        })
    );
});
