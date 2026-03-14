cmake_minimum_required(VERSION 3.14) 
project(nifl C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

message(STATUS "Fetching libui-ng...")
FetchContent_Declare(
    libui_ng
    GIT_REPOSITORY https://github.com/libui-ng/libui-ng.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(libui_ng)

# --- MESON BUILD INTEGRATION ---
set(LIBUI_OUT_DIR "${libui_ng_BINARY_DIR}/meson-build")

# 1. Configure Meson
add_custom_command(
    OUTPUT "${LIBUI_OUT_DIR}/build.ninja"
    COMMAND meson setup "${LIBUI_OUT_DIR}" "${libui_ng_SOURCE_DIR}" --buildtype=release --default-library=shared
    COMMENT "Configuring libui-ng with Meson..."
)

# 2. Compile with Meson
# FIX: We use a .marker file so Make tracks completion perfectly, ignoring symlinks
add_custom_command(
    OUTPUT "${LIBUI_OUT_DIR}/libui_compile.marker"
    COMMAND meson compile -C "${LIBUI_OUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E touch "${LIBUI_OUT_DIR}/libui_compile.marker"
    DEPENDS "${LIBUI_OUT_DIR}/build.ninja"
    COMMENT "Compiling libui-ng with Meson..."
)

# 3. Create a target that MyGUIApp can depend on
add_custom_target(libui_build DEPENDS "${LIBUI_OUT_DIR}/libui_compile.marker")

# --- MUI LIBRARY ---
add_subdirectory(mui)

# --- YOUR EXECUTABLE ---
add_executable(${PROJECT_NAME} main.cpp)

# Link your app against the mui library.
# This will transitively link against libui and set up include directories.
target_link_libraries(${PROJECT_NAME} PRIVATE mui)

# FIX: Tell the linker to search both possible Meson output directories
target_link_directories(${PROJECT_NAME} PRIVATE 
    "${LIBUI_OUT_DIR}"
    "${LIBUI_OUT_DIR}/meson-out"
)

# Ensure your app can find libui.so at runtime when testing locally
set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "${LIBUI_OUT_DIR}:${LIBUI_OUT_DIR}/meson-out")