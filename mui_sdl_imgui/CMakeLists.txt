cmake_minimum_required(VERSION 3.20)
project(mui_sdl_imgui)

find_package(SDL3 REQUIRED)

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

add_library(mui STATIC
    app.cpp
    control.cpp
    window.cpp
    button.cpp
    checkbox.cpp
    box.cpp
    grid.cpp
    tab.cpp
    entry.cpp
    spinbox.cpp
    slider.cpp
    progressbar.cpp
    dialogs.cpp
    label.cpp
    group.cpp
)

# Assumes existing .hpp files are in the same directory or provided by the parent
target_include_directories(mui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(mui PRIVATE 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_sources(mui PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_link_libraries(mui PUBLIC SDL3::SDL3)
