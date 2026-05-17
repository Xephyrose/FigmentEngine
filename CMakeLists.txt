cmake_minimum_required(VERSION 4.2)
project(FigmentEngine)

set(CMAKE_CXX_STANDARD 26)

find_package(SDL3 CONFIG REQUIRED)

add_executable(FigmentEngine
        main.cpp
        Node.cpp
        Node.h
        Vector2.cpp
        Vector2.h
)

target_link_libraries(FigmentEngine PRIVATE SDL3::SDL3)