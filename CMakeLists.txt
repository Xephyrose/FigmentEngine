cmake_minimum_required(VERSION 3.20...4.2)
project(FigmentEngine)

set(CMAKE_CXX_STANDARD 26)

find_package(SDL3 CONFIG REQUIRED)

find_package(SDL3_image REQUIRED)

add_executable(FigmentEngine
        main.cpp
        Node.cpp
        Mesh.cpp
        Vertex.cpp
        Transform3D.cpp
        Transform3D.h
        Node3D.cpp
        Node3D.h
        Camera3D.cpp
        Camera3D.h
)

target_include_directories(FigmentEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(FigmentEngine PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        ${CMAKE_DL_LIBS}
        pthread
)