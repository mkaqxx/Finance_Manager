#include "core/api_handler.h"
#include <windows.h>
#include <filesystem>

int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    Finance_manager manager;
    webview::webview w(true, nullptr);
    w.set_title("Finance Manager");
    w.set_size(1280, 800, WEBVIEW_HINT_NONE);

    // Находим окно напрямую через Windows API по его заголовку (который мы задали строкой выше)
    HWND hwnd = FindWindowA(nullptr, "Finance Manager");
    HINSTANCE hInst = GetModuleHandle(nullptr);

    // Убрали букву 'L' перед строкой "MAINICON", теперь типы полностью совпадают
    HICON hIconLarge = (HICON)LoadImage(hInst, "MAINICON", IMAGE_ICON,
                                        GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
    HICON hIconSmall = (HICON)LoadImage(hInst, "MAINICON", IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

    // Устанавливаем иконки окну
    if (hIconLarge) SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);
    if (hIconSmall) SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

    // Устанавливаем иконки окну
    if (hIconLarge) SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);
    if (hIconSmall) SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

    ApiHandler api(manager, w);
    api.register_all();


    std::filesystem::path html_path = std::filesystem::current_path() / ".." / "assets" / "index.html";
    if (!std::filesystem::exists(html_path)) {
         html_path = std::filesystem::current_path() / "assets" / "index.html";
    }
    if (std::filesystem::exists(html_path)) {
        html_path = std::filesystem::canonical(html_path);
    }
    w.navigate("file:///" + html_path.generic_string());
    w.run();
    return 0;
}