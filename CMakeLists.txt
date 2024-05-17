cmake_minimum_required(VERSION 3.22)
project(TicketSystem)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
set(SRC_LIST main.cpp)
include_directories(include)
add_executable(${PROJECT_NAME} ${SRC_LIST})

add_subdirectory(src)