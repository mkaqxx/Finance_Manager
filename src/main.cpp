#include "core/api_handler.h"
#include <windows.h>
#include <filesystem>

int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    Finance_manager manager;
    webview::webview w(true, nullptr);
    w.set_title("Finance Manager");
    w.set_size(1280, 800, WEBVIEW_HINT_NONE);

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