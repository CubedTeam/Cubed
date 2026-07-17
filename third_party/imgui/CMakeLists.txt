cmake_minimum_required(VERSION 3.15)

project(imui)

set(IMGUI_SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_impl_sdl3.cpp
    imgui_impl_opengl3.cpp
    misc/freetype/imgui_freetype.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    imgui
    PUBLIC
    SDL3::SDL3
    OpenGL::GL
    Freetype::Freetype
)