#include "core/api_handler.h"
#include <iostream>
#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Глобальные экземпляры для среды WebAssembly
static std::unique_ptr<Finance_manager> g_manager;
static std::unique_ptr<ApiHandler> g_api;

int main() {
    std::cout << "[WASM] Finance Manager WebAssembly core initialized.\n";
    g_manager = std::make_unique<Finance_manager>();
    g_api = std::make_unique<ApiHandler>(*g_manager);
    return 0;
}