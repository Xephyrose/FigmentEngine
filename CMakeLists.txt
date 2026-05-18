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
)

target_link_libraries(FigmentEngine PRIVATE SDL3::SDL3)

# Macro for Compiling Shaders (will be called in each chapter's CMakeLists.txt)
macro(compile_shaders)
    set(SHADERS_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)


    file(GLOB_RECURSE src_shader_files
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/
            "${CMAKE_CURRENT_SOURCE_DIR}/*.hlsl")


    # Find folders that could contain shadercross
    file(GLOB SC_DIRS "${CMAKE_SOURCE_DIR}/SDL*_shadercross*")
    find_program(SC_EXECUTABLE shadercross REQUIRED
            HINTS ${SC_DIRS}
            PATH_SUFFIXES bin)


    foreach (fn ${src_shader_files})
        set(in_f "${CMAKE_CURRENT_SOURCE_DIR}/${fn}")
        set(out_f "${SHADERS_OUTPUT_DIR}/${fn}")
        string(REPLACE ".hlsl" ".spv" fn_spv ${out_f})
        string(REPLACE ".hlsl" ".msl" fn_msl ${out_f})
        string(REPLACE ".hlsl" ".dxil" fn_dxil ${out_f})
        add_custom_command(
                OUTPUT ${out_f}
                COMMAND ${SC_EXECUTABLE} ${in_f} -o ${fn_spv}
                COMMAND ${SC_EXECUTABLE} ${in_f} -o ${fn_msl}
                COMMAND ${SC_EXECUTABLE} ${in_f} -o ${fn_dxil}
                MAIN_DEPENDENCY ${in_f}
        )
        list(APPEND out_shader_files ${out_f})
    endforeach ()


    add_custom_target(${PROJECT_NAME}_shaders DEPENDS ${out_shader_files})


    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)
endmacro()