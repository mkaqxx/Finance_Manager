cmake_minimum_required(VERSION 3.14)
project(FinanceManager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(webview)


add_executable(FinanceManager WIN32
        src/main.cpp
        src/models/transaction.cpp
        src/models/account.cpp
        src/models/budget.cpp
        src/core/storage.cpp
        src/core/finance_manager.cpp
        src/core/statistics.cpp
        src/core/report.cpp
        src/utils/chrono_to_string.cpp)

target_include_directories(FinanceManager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(FinanceManager PRIVATE webview)
