cmake_minimum_required(VERSION 3.20...4.2)
project(FigmentEngine)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

set(SDL3_DIR "${sdl3_BINARY_DIR}" CACHE PATH "Path to built SDL3 config" FORCE)

FetchContent_Declare(
        SDL3_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG main
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3_image)

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
        FreeCam3D.cpp
        FreeCam3D.h
        AppState.h
        MaterialUnlitTextured.cpp
        MaterialUnlitTextured.h
        Material.h
        AppState.cpp
        MaterialUnlitTextured.cpp
        MeshInstance3D.cpp
        MeshInstance3D.h
        imgui.cpp
        imgui.h
        imgui_impl_sdl3.cpp
        imgui_impl_sdl3.h
        imgui_impl_sdlgpu3.cpp
        imgui_impl_sdlgpu3.h
        imgui_impl_sdlgpu3_shaders.h
        imconfig.h
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_internal.h
        imgui_tables.cpp
        imgui_widgets.cpp
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        Input.cpp
        Input.h
        imgui_stdlib.cpp
        imgui_stdlib.h
        Node2D.cpp
        Node2D.h
        Transform2D.cpp
        Transform2D.h
        Camera2D.cpp
        Camera2D.h
        Sprite2D.cpp
        Sprite2D.h
)

target_include_directories(FigmentEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(FigmentEngine PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        ${CMAKE_DL_LIBS}
        pthread
)

add_custom_command(TARGET FigmentEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:FigmentEngine>
        $<TARGET_FILE_DIR:FigmentEngine>
        COMMAND_EXPAND_LISTS
        COMMENT "Deploying runtime DLLs"
)

if(MINGW)
    find_program(GCC_EXECUTABLE gcc)
    if(GCC_EXECUTABLE)
        get_filename_component(GCC_BIN_DIR ${GCC_EXECUTABLE} DIRECTORY)
        add_custom_command(TARGET FigmentEngine POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${GCC_BIN_DIR}/libgcc_s_seh-1.dll"
                "${GCC_BIN_DIR}/libstdc++-6.dll"
                $<TARGET_FILE_DIR:FigmentEngine>
                COMMENT "Deploying MinGW runtime DLLs"
        )
    endif()
endif()

if(WIN32)
    set(SHADERCROSS_HINT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/SDL3_shadercross-3.0.0-windows-mingw-x64/bin/")
elseif(UNIX AND NOT APPLE)
    set(SHADERCROSS_HINT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/SDL3_shadercross-3.0.0-linux-x64/bin/")
else()
    message(FATAL_ERROR "Unsupported platform for shadercross tool.")
endif()

find_program(SHADERCROSS_EXECUTABLE
        NAMES shadercross
        PATHS ${SHADERCROSS_HINT_PATH}
        NO_DEFAULT_PATH
        DOC "Path to the SDL shadercross CLI tool"
)

if(SHADERCROSS_EXECUTABLE)
    message(STATUS "Found shadercross: ${SHADERCROSS_EXECUTABLE}")

    set(SHADERS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders)
    set(SHADERS_BINARY_DIR ${CMAKE_BINARY_DIR}/assets/shaders)
    set(ASSETS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    set(ASSETS_BINARY_DIR ${CMAKE_BINARY_DIR}/assets)

    add_custom_target(CopyAssets ALL
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${ASSETS_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SOURCE_DIR} ${ASSETS_BINARY_DIR}
            COMMENT "Copying entire assets folder to build directory"
    )

    add_custom_target(CompileShaders ALL
            COMMENT "Compiling shaders..."
            DEPENDS CopyAssets
    )

    function(compile_shader HLSL_FILE)
        get_filename_component(SHADER_FULLNAME ${HLSL_FILE} NAME)

        # Strip only the .hlsl extension, preserving the .vert or .frag
        string(REGEX REPLACE "\\.hlsl$" "" SHADER_NAME ${SHADER_FULLNAME})

        # Determine shader stage from filename
        if(SHADER_FULLNAME MATCHES "\\.vert\\.")
            set(SHADER_STAGE "vertex")
        elseif(SHADER_FULLNAME MATCHES "\\.frag\\.")
            set(SHADER_STAGE "fragment")
        else()
            set(SHADER_STAGE "vertex") # default fallback
            message(WARNING "Could not determine shader stage for ${SHADER_FULLNAME}, defaulting to vertex")
        endif()

        set(OUTPUTS)
        list(APPEND OUTPUTS ${SHADERS_BINARY_DIR}/${SHADER_NAME}.spv)
        list(APPEND OUTPUTS ${SHADERS_BINARY_DIR}/${SHADER_NAME}.dxil)
        list(APPEND OUTPUTS ${SHADERS_BINARY_DIR}/${SHADER_NAME}.msl)

        add_custom_command(
                OUTPUT ${OUTPUTS}
                COMMAND ${SHADERCROSS_EXECUTABLE} "$<GENEX_EVAL:${SHADERS_BINARY_DIR}/${SHADER_FULLNAME}>" --stage ${SHADER_STAGE} -o ${SHADERS_BINARY_DIR}/${SHADER_NAME}.spv
                COMMAND ${SHADERCROSS_EXECUTABLE} "$<GENEX_EVAL:${SHADERS_BINARY_DIR}/${SHADER_FULLNAME}>" --stage ${SHADER_STAGE} -o ${SHADERS_BINARY_DIR}/${SHADER_NAME}.dxil
                COMMAND ${SHADERCROSS_EXECUTABLE} "$<GENEX_EVAL:${SHADERS_BINARY_DIR}/${SHADER_FULLNAME}>" --stage ${SHADER_STAGE} -o ${SHADERS_BINARY_DIR}/${SHADER_NAME}.msl
                DEPENDS CopyAssets
                COMMENT "Compiling ${SHADER_FULLNAME} (${SHADER_STAGE}) to spv, dxil, and msl"
        )

        string(REPLACE "." "_" SAFE_TARGET_NAME ${SHADER_NAME})
        add_custom_target(Compile_${SAFE_TARGET_NAME} DEPENDS ${OUTPUTS})
        add_dependencies(CompileShaders Compile_${SAFE_TARGET_NAME})
    endfunction()

    # Find all .hlsl files
    file(GLOB HLSL_FILES ${SHADERS_SOURCE_DIR}/*.hlsl)

    # Compile
    foreach(HLSL_FILE ${HLSL_FILES})
        compile_shader(${HLSL_FILE})
    endforeach()

    if(HLSL_FILES)
        message(STATUS "Found HLSL shader files to compile:")
        foreach(HLSL_FILE ${HLSL_FILES})
            message(STATUS "  ${HLSL_FILE}")
        endforeach()
    else()
        message(STATUS "No .hlsl files found in ${SHADERS_SOURCE_DIR}")
    endif()

    add_dependencies(FigmentEngine CompileShaders)
else()
    message(WARNING "Shadercross executable not found in /tools/, skipping shader compilation.")
endif()