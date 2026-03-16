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

set(CORE_SOURCES core/app.cpp core/window.cpp core/theme.cpp)

set(WIDGET_SOURCES
    widgets/button.cpp
    widgets/card.cpp
    widgets/checkbox.cpp
    widgets/control.cpp
    widgets/entry.cpp
    widgets/label.cpp
    widgets/progressbar.cpp
    widgets/slider.cpp
    widgets/spinbox.cpp
    widgets/image.cpp
    widgets/coloredit.cpp
    widgets/combobox.cpp
    widgets/separator.cpp
    widgets/radiobutton.cpp)

set(LAYOUT_SOURCES layouts/box.cpp layouts/grid.cpp layouts/group.cpp
                   layouts/tab.cpp layouts/treenode.cpp)

set(DIALOG_SOURCES
    dialogs/ImFileDialog.cpp
    dialogs/dialogs.cpp
    dialogs/mui_breadcrumb_bar.cpp
    dialogs/mui_file_dialog.cpp
    dialogs/mui_file_dialog_layouts.cpp
    dialogs/mui_icon_grid.cpp
    dialogs/stb_image.cpp
    dialogs/mui_tree_view.cpp)

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
