cmake_minimum_required(VERSION 3.16)
project(DungeonCrawler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(CONFIGPARSER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/configparser"
    CACHE PATH "Path to the multi-format config parser sources")

add_subdirectory(${CONFIGPARSER_DIR} configparser_build)

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(dungeon
    src/main.cpp
    src/Game.cpp
    src/Map.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Entity.cpp
    src/Item.cpp
    src/Inventory.cpp
    src/LootTable.cpp
    src/Renderer.cpp
    src/Menu.cpp
)
target_include_directories(dungeon PRIVATE src)
target_link_libraries(dungeon PRIVATE raylib configparser)

if(MSVC)
    target_link_options(dungeon PRIVATE
        $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
        $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
endif()

add_custom_command(TARGET dungeon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/configs $<TARGET_FILE_DIR:dungeon>/configs)

install(TARGETS dungeon RUNTIME DESTINATION .)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/configs DESTINATION .)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "ShadowDungeon")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_FILE_NAME "ShadowDungeon-1.0.0-win64")
include(CPack)