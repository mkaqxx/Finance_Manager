#include "core/api_handler.h"
#include <windows.h>
#include <filesystem>


//главная точка входа приложения win32
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    //создание объекта финансового менеджера
    Finance_manager manager;
    //создание окна webview с включенным контекстным меню
    webview::webview w(true, nullptr);
    //установка заголовка окна
    w.set_title("Finance Manager");
    //установка начального размера окна
    w.set_size(1280, 800, WEBVIEW_HINT_NONE);

    //получение дескриптора окна
    HWND hwnd = nullptr;
    auto win_res = w.window();
    if (win_res.has_value()) {
        hwnd = static_cast<HWND>(win_res.value());
    }
    //запасной поиск окна по заголовку
    if (!hwnd) {
        hwnd = FindWindowA(nullptr, "Finance Manager");
    }
    //получение дескриптора модуля
    HINSTANCE hInst = GetModuleHandle(nullptr);

    //загрузка большой и малой иконок из ресурсов
    HICON hIconLarge = (HICON)LoadImage(hInst, "MAINICON", IMAGE_ICON,
                                        GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
    HICON hIconSmall = (HICON)LoadImage(hInst, "MAINICON", IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

    //установка иконок для окна приложения
    if (hwnd) {
        if (hIconLarge) SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);
        if (hIconSmall) SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    }

    //создание обработчика api и регистрация всех биндингов
    ApiHandler api(manager, w);
    api.register_all();

    //буфер для пути к исполняемому файлу
    wchar_t exeBuffer[MAX_PATH];
    std::filesystem::path exeDir;
    //определение директории исполняемого файла
    if (GetModuleFileNameW(nullptr, exeBuffer, MAX_PATH) > 0) {
        exeDir = std::filesystem::path(exeBuffer).parent_path();
    } else {
        exeDir = std::filesystem::current_path();
    }

    //поиск пути к index.html в возможных расположениях
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

    //приведение пути к каноническому виду
    if (std::filesystem::exists(html_path)) {
        html_path = std::filesystem::canonical(html_path);
    }
    //загрузка страницы в webview
    w.navigate("file:///" + html_path.generic_string());
    //запуск главного цикла приложения
    w.run();
    return 0;
}
