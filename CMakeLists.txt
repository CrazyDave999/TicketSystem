cmake_minimum_required(VERSION 3.22)
project(TicketSystem)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast")
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
include_directories(include include/data_structures include/data_structures/BPT/include)
add_executable(${PROJECT_NAME} ${SRC_LIST})
add_subdirectory(include/data_structures/BPT/src)