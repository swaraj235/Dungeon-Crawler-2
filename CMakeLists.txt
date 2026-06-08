cmake_minimum_required(VERSION 3.15)
project(DungeonCrawler2 VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Raylib (works on Linux/macOS with system-installed raylib)
find_package(raylib QUIET)

if(NOT raylib_FOUND)
    # Fallback: try pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(RAYLIB QUIET raylib)
    endif()

    if(NOT RAYLIB_FOUND)
        # Windows fallback: hardcoded path
        if(WIN32)
            set(RAYLIB_PATH "C:/Users/NILESH/Downloads/raylib-5.5_win64_mingw-w64/raylib")
            set(RAYLIB_INCLUDE_DIRS "${RAYLIB_PATH}/include")
            set(RAYLIB_LIBRARY_DIRS "${RAYLIB_PATH}/lib")
            link_directories(${RAYLIB_LIBRARY_DIRS})
        else()
            message(FATAL_ERROR "Raylib not found! Install with: sudo apt-get install libraylib-dev")
        endif()
    endif()
endif()

# Include directories
include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/Core
        ${PROJECT_SOURCE_DIR}/include/Entities
        ${PROJECT_SOURCE_DIR}/include/Systems
        ${PROJECT_SOURCE_DIR}/include/Audio
        ${PROJECT_SOURCE_DIR}/include/UI
        ${PROJECT_SOURCE_DIR}/include/external
        ${RAYLIB_INCLUDE_DIRS}
)

# Collect all source files
file(GLOB_RECURSE SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/main.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries (platform-specific)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} raylib opengl32 gdi32 winmm)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} raylib "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
else()
    # Linux
    target_link_libraries(${PROJECT_NAME} raylib GL m pthread dl rt X11)
endif()

# Copy assets folder to build directory after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# Copy saves folder to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/saves
)

# Copy web_dashboard folder to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/web_dashboard
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/web_dashboard
)

# Set output directory for executable
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Print configuration info
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

