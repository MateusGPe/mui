cmake_minimum_required(VERSION 3.20)
project(nifl C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- MUI LIBRARY (SDL3 + ImGui) ---
# This subdirectory will automatically fetch ImGui and find SDL3.
add_subdirectory(mui)

# --- YOUR EXECUTABLE ---
add_executable(${PROJECT_NAME} main.cpp)

# Link your app against the mui library.
target_link_libraries(${PROJECT_NAME} PUBLIC mui)

# WORKAROUND: The 'mui' library does not correctly export the include
# directory for its dependency (ImGui). We manually add it to our
# executable target here. We also add the mui library source as an include
# path to find its headers.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/_deps/imgui-src
    mui
)

# --- THEME EDITOR EXECUTABLE ---
add_executable(theme_editor theme_edit.cpp)

# Link theme editor against the mui library.
target_link_libraries(theme_editor PUBLIC mui)

# Add include directories for the theme editor.
target_include_directories(theme_editor PRIVATE
    ${CMAKE_BINARY_DIR}/_deps/imgui-src
    mui
)

# --- THEME EDITOR EXECUTABLE ---
add_executable(sample sample.cpp)

# Link theme editor against the mui library.
target_link_libraries(sample PUBLIC mui)

# Add include directories for the theme editor.
target_include_directories(sample PRIVATE
    ${CMAKE_BINARY_DIR}/_deps/imgui-src
    mui
)