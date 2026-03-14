# NOTE: Using file(GLOB) is convenient but is not recommended for general use.
# If you add or remove source files, you may need to re-run CMake manually.
# For more robust projects, list all source files explicitly.
file(GLOB MUI_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

add_library(mui STATIC ${MUI_SOURCES})

# Ensure libui-ng is built before this library
add_dependencies(mui libui_build)

# Setup include directories for the library and its consumers.
target_include_directories(mui
    # PUBLIC: Needed by mui's implementation AND by consumers, because
    # mui headers include <ui.h>.
    PUBLIC
        "${libui_ng_SOURCE_DIR}"

    # PRIVATE: Needed only for compiling mui's own source files,
    # which use includes like "window.hpp".
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"

    # INTERFACE: Needed only by consumers. Consumers use #include "mui/header.hpp",
    # so their include path needs to contain the parent directory of `mui`.
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

# Since mui is a static library, its link dependencies must be propagated
# to the final target that links against it.
target_link_libraries(mui PUBLIC ui)