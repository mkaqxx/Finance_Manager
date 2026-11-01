#include "core/api_handler.h"
#include <windows.h>
#include <filesystem>

// Точка входа в программу
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    Finance_manager manager;
    // Создаем окно
    webview::webview w(true, nullptr);
    w.set_title("Finance Manager");
    w.set_size(1280, 800, WEBVIEW_HINT_NONE);

    // Получаем хендл окна
    HWND hwnd = nullptr;
    auto win_res = w.window();
    if (win_res.has_value()) {
        hwnd = static_cast<HWND>(win_res.value());
    }
    // Если не вышло, ищем окно по заголовку
    if (!hwnd) {
        hwnd = FindWindowA(nullptr, "Finance Manager");
    }
    HINSTANCE hInst = GetModuleHandle(nullptr);

    // Грузим иконки
    HICON hIconLarge = (HICON)LoadImage(hInst, "MAINICON", IMAGE_ICON,
                                        GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
    HICON hIconSmall = (HICON)LoadImage(hInst, "MAINICON", IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

    // Ставим иконки окну
    if (hwnd) {
        if (hIconLarge) SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);
        if (hIconSmall) SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    }

    // Регистрируем API
    ApiHandler api(manager, w);
    api.register_all();

    // Ищем путь к HTML
    wchar_t exeBuffer[MAX_PATH];
    std::filesystem::path exeDir;
    // Ищем относительно exe
    if (GetModuleFileNameW(nullptr, exeBuffer, MAX_PATH) > 0) {
        exeDir = std::filesystem::path(exeBuffer).parent_path();
    } else {
        exeDir = std::filesystem::current_path();
    }

    // Проверяем где лежит HTML
    std::filesystem::path html_path = exeDir / "assets" / "index.html";
    if (!std::filesystem::exists(html_path)) {
        html_path = exeDir / ".." / "assets" / "index.html";
    }
    if (!std::filesystem::exists(html_path)) {
        html_path = std::filesystem::current_path() / "assets" / "index.html";
    }
    if (!std::filesystem::exists(html_path)) {
        html_path = std::filesystem::current_path() / ".." / "assets" / "index.html";
    }

    // Приводим путь к нормальному виду
    if (std::filesystem::exists(html_path)) {
        html_path = std::filesystem::canonical(html_path);
    }
    // Открываем HTML и запускаем цикл
    w.navigate("file:///" + html_path.generic_string());
    w.run();
    return 0;
}
