cmake_minimum_required(VERSION 4.2)
project(src)

set(CMAKE_CXX_STANDARD 20)

add_executable(src main.cpp
        models/transaction.h
        models/transaction.cpp
        models/account.h
        models/account.cpp
        models/budget.h
        models/budget.cpp
        storage.h
        finance_manager.h
        storage.cpp
        finance_manager.cpp)
