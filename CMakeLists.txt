cmake_minimum_required(VERSION 3.16)
project(DungeonCrawler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CONFIGPARSER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/configparser"
    CACHE PATH "Path to the multi-format config parser sources")

add_subdirectory(${CONFIGPARSER_DIR} configparser_build)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(dungeon
    src/main.cpp
    src/Game.cpp
    src/Map.cpp
    src/Player.cpp
    src/Renderer.cpp
)
target_include_directories(dungeon PRIVATE src)
target_link_libraries(dungeon PRIVATE raylib configparser)

add_custom_command(TARGET dungeon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/configs $<TARGET_FILE_DIR:dungeon>/configs)