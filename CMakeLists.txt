cmake_minimum_required(VERSION 3.14)
project(FinanceManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Автоматически скачиваем WebView2 SDK напрямую от Microsoft
include(FetchContent)
FetchContent_Declare(
        webview2_sdk
        URL https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(webview2_sdk)

# 2. Исходники проекта
add_executable(FinanceManager WIN32
        src/main.cpp
        src/models/transaction.cpp
        src/models/account.cpp
        src/models/budget.cpp
        src/core/storage.cpp
        src/core/finance_manager.cpp
        src/core/statistics.cpp
        src/core/report.cpp
        src/utils/chrono_to_string.cpp
        src/core/api_handler.h
        src/core/api_handler.cpp
)

# 3. Пути к заголовочным файлам (включая скачанный SDK)
target_include_directories(FinanceManager PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/webview/core/include
        ${webview2_sdk_SOURCE_DIR}/build/native/include
)

# 4. Линкуем статический SDK и системные библиотеки Windows
target_link_libraries(FinanceManager PRIVATE
        ${webview2_sdk_SOURCE_DIR}/build/native/x64/WebView2LoaderStatic.lib
        version
        advapi32
        ole32
        shell32
        shlwapi
        user32
)