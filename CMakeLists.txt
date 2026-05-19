cmake_minimum_required(VERSION 3.20...4.2)
project(FigmentEngine)

set(CMAKE_CXX_STANDARD 26)

find_package(SDL3 CONFIG REQUIRED)

find_package(SDL3 CONFIG REQUIRED)
message(STATUS "SDL3 found at: ${SDL3_DIR}")
message(STATUS "SDL3 libraries: ${SDL3_LIBRARIES}")

add_executable(FigmentEngine
        main.cpp
        Node.cpp
        Node.h
        Vector2.cpp
        Vector2.h
        Vector3.cpp
        Vector3.h
        Game.cpp
        Game.h
        Mesh.cpp
        Mesh.h
        Vertex.cpp
        Vertex.h
)

target_link_libraries(FigmentEngine PRIVATE SDL3::SDL3)