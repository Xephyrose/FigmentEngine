cmake_minimum_required(VERSION 3.20...4.2)
project(FigmentEngine)

set(CMAKE_CXX_STANDARD 26)

find_package(SDL3 CONFIG REQUIRED)

find_package(SDL3_image REQUIRED)

add_executable(FigmentEngine
        main.cpp
        Node.cpp
        Vector2.cpp
        Vector3.cpp
        Mesh.cpp
        Vertex.cpp
)

target_include_directories(FigmentEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(FigmentEngine PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        ${CMAKE_DL_LIBS}
        pthread
)