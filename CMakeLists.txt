cmake_minimum_required(VERSION 3.20)
project(nifl C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- MUI LIBRARY (SDL3 + ImGui) ---
# This subdirectory will automatically fetch ImGui and find SDL3.
add_subdirectory(mui)

# Make the 'mui' library usable from a parent project by adding its
# required include directories to its public interface. This fixes the
# issue described in the WORKAROUND in the original file.
target_include_directories(mui INTERFACE
    # Add the mui source directory to find its headers (e.g., mui.hpp)
    ${CMAKE_CURRENT_SOURCE_DIR}/mui
    # Add the fetched ImGui dependency's source directory
    ${CMAKE_BINARY_DIR}/_deps/imgui-src
)

# --- EXAMPLES ---
# Only build the example executables if this project is being built directly,
# not when it's included as a subdirectory in another project.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # --- Main Gallery Example ---
    add_executable(${PROJECT_NAME} main.cpp)
    target_link_libraries(${PROJECT_NAME} PUBLIC mui)

    # --- Theme Editor Example ---
    add_executable(theme_editor theme_edit.cpp)
    target_link_libraries(theme_editor PUBLIC mui)

    # --- Sample App Example ---
    add_executable(sample sample.cpp)
    target_link_libraries(sample PUBLIC mui)
endif()