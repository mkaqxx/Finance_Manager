    cmake_minimum_required(VERSION 3.14)
    project(FinanceManager LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(COMMON_SOURCES
            src/main.cpp
            src/models/transaction.cpp
            src/models/account.cpp
            src/models/budget.cpp
            src/core/storage.cpp
            src/core/finance_manager.cpp
            src/core/statistics.cpp
            src/core/report.cpp
            src/utils/chrono_to_string.cpp
            src/core/api_handler.cpp
            src/utils/safe_open_file.cpp
    )

    if (EMSCRIPTEN)
        add_executable(FinanceManager ${COMMON_SOURCES})

        target_include_directories(FinanceManager PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

        # Флаги линковщика Emscripten:
        target_link_options(FinanceManager PRIVATE
                -lembind
                -lidbfs.js
                -sALLOW_MEMORY_GROWTH=1
                -sNO_EXIT_RUNTIME=1
                -sMODULARIZE=1
                -sEXPORT_NAME="createFinanceManagerModule"
                -sEXPORTED_RUNTIME_METHODS=['FS','IDBFS']
        )

        # Генерация .js и .wasm в директорию assets/wasm/
        set_target_properties(FinanceManager PROPERTIES
                OUTPUT_NAME "finance_manager"
                SUFFIX ".js"
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/wasm"
        )
    else()
        # Десктопная сборка Windows WebView2 (если потребуется параллельная поддержка)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        include(FetchContent)
        FetchContent_Declare(
                webview2_sdk
                URL https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2
                DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(webview2_sdk)

        add_executable(FinanceManager WIN32 ${COMMON_SOURCES} src/resources.rc)
        target_include_directories(FinanceManager PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/webview/core/include
                ${webview2_sdk_SOURCE_DIR}/build/native/include
        )
        target_link_libraries(FinanceManager PRIVATE
                ${webview2_sdk_SOURCE_DIR}/build/native/x64/WebView2LoaderStatic.lib
                version advapi32 ole32 shell32 shlwapi user32
        )
    endif()