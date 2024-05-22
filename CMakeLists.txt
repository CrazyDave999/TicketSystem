cmake_minimum_required(VERSION 3.22)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
set(
        SRC_LIST
        main.cpp
        src/account/account.cpp
        src/common/management_system.cpp
        src/common/string_utils.cpp
        src/common/utils.cpp
        src/train/queue_system.cpp
        src/train/train.cpp
)
include_directories(include)
add_executable(${PROJECT_NAME} ${SRC_LIST})