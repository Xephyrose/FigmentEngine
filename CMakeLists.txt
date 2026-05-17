cmake_minimum_required(VERSION 4.2)
project(FigmentEngine)

set(CMAKE_CXX_STANDARD 26)

add_executable(FigmentEngine main.cpp
        Node.cpp
        Node.h
        Vector2.cpp
        Vector2.h
)
