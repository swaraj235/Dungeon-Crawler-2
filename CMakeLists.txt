cmake_minimum_required(VERSION 3.15)
project(DungeonCrawler2 VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Raylib path (hardcoded for your setup)
set(RAYLIB_PATH "C:/Users/NILESH/Downloads/raylib-5.5_win64_mingw-w64/raylib")

# Include and library directories
include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/Core
        ${PROJECT_SOURCE_DIR}/include/Entities
        ${PROJECT_SOURCE_DIR}/include/Systems
        ${PROJECT_SOURCE_DIR}/include/Audio
        ${PROJECT_SOURCE_DIR}/include/UI
        ${PROJECT_SOURCE_DIR}/include/external
        ${RAYLIB_PATH}/include
)

link_directories(
        ${RAYLIB_PATH}/lib
)

# Collect all source files
file(GLOB_RECURSE SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/Systems/EffectSystem.cpp"
        "${PROJECT_SOURCE_DIR}/src/Systems/ItemSystem.cpp"
        "${PROJECT_SOURCE_DIR}/src/Systems/CompanionSystem.cpp"
        "${PROJECT_SOURCE_DIR}/src/UI/MainMenu.cpp"
        "${PROJECT_SOURCE_DIR}/main.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link Raylib and Windows libraries
target_link_libraries(${PROJECT_NAME}
        raylib
        opengl32
        gdi32
        winmm
)

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
message(STATUS "Raylib Path: ${RAYLIB_PATH}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

