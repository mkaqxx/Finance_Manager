cmake_minimum_required(VERSION 3.14)
project(FinanceManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Вшиваем стандартную библиотеку C++ внутрь .exe для портативности на чужих ПК
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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
        src/utils/safe_open_file.h
        src/utils/safe_open_file.cpp
        src/resources.rc
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
# ==============================================================================
# 5. ТЕСТИРОВАНИЕ: Подключение GoogleTest через FetchContent
# ==============================================================================
include(FetchContent)

# 1. Объявляем, откуда CMake должен скачать GoogleTest (официальный релиз)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Для Windows (MSVC): заставляем GTest использовать ту же runtime-библиотеку CRT
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# 2. Скачиваем и распаковываем GoogleTest прямо во время конфигурации CMake
FetchContent_MakeAvailable(googletest)

# 3. Включаем поддержку тестов в CMake (для CTest)
enable_testing()

# 4. Создаем отдельный консольный exe-файл с тестами
add_executable(FinanceTests
        # Файлы самих тестов:
        tests/test_accounts.cpp
        tests/test_transactions.cpp
        tests/test_finance_manager.cpp
        tests/test_statistics.cpp

        # Файлы логики вашего проекта (без main.cpp и api_handler.cpp, чтобы не зависеть от окна UI):
        src/models/account.cpp
        src/models/transaction.cpp
        src/models/budget.cpp
        src/core/finance_manager.cpp
        src/core/storage.cpp
        src/core/statistics.cpp
        src/core/report.cpp
        src/utils/chrono_to_string.cpp
        src/utils/safe_open_file.cpp
)

# 5. Указываем путь к исходникам (чтобы работали инклюды вида #include "models/account.h")
target_include_directories(FinanceTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 6. Линкуем готовый модуль GTest::gtest_main (он сам предоставляет функцию main() для запуска тестов!)
target_link_libraries(FinanceTests PRIVATE
        GTest::gtest_main
)

# 7. Автоматически регистрируем каждый тест в CTest и в IDE
include(GoogleTest)
gtest_discover_tests(FinanceTests)