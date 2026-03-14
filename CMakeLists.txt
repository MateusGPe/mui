cmake_minimum_required(VERSION 3.20)
project(nifl C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- MUI LIBRARY (SDL3 + ImGui) ---
# This subdirectory will automatically fetch ImGui and find SDL3.
add_subdirectory(mui_sdl_imgui)

# --- YOUR EXECUTABLE ---
add_executable(${PROJECT_NAME} main.cpp)

# Link your app against the mui library.
target_link_libraries(${PROJECT_NAME} PRIVATE mui)