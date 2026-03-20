cmake_minimum_required(VERSION 3.20)
project(mui_sdl_imgui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---
find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

if(UNIX AND NOT APPLE)
  find_package(Fontconfig)
endif()

include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking)
FetchContent_MakeAvailable(imgui)

# --- Source Definitions ---

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "core/*.cpp")
file(GLOB WIDGET_SOURCES CONFIGURE_DEPENDS "widgets/*.cpp")
file(GLOB LAYOUT_SOURCES CONFIGURE_DEPENDS "layouts/*.cpp")
# Using GLOB_RECURSE for dialogs to include sources in subdirectories.
# CONFIGURE_DEPENDS ensures CMake re-runs if files are added or removed.
file(GLOB_RECURSE DIALOG_SOURCES CONFIGURE_DEPENDS "dialogs/*.cpp")

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp)

# --- Target Configuration ---

add_library(mui STATIC ${CORE_SOURCES} ${WIDGET_SOURCES} ${LAYOUT_SOURCES}
                       ${DIALOG_SOURCES} ${IMGUI_SOURCES})

# Include paths for your code and ImGui
target_include_directories(
  mui
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core
         ${CMAKE_CURRENT_SOURCE_DIR}/widgets
         ${CMAKE_CURRENT_SOURCE_DIR}/layouts
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         ${CMAKE_CURRENT_SOURCE_DIR}/dialogs # Add dialogs folder for headers
)

target_include_directories(mui PRIVATE ${imgui_SOURCE_DIR}
                                       ${imgui_SOURCE_DIR}/backends)

# --- Linking ---
target_link_libraries(mui PUBLIC SDL3::SDL3 OpenGL::GL)

if(Fontconfig_FOUND)
  target_link_libraries(mui PRIVATE Fontconfig::Fontconfig)
endif()
