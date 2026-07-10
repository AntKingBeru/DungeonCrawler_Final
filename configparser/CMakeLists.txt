cmake_minimum_required(VERSION 3.16)
project(configparser LANGUAGES CXX)

add_library(configparser STATIC
    StringUtils.cpp
    IniParser.cpp
    JsonParser.cpp
    XmlParser.cpp
    ParserFactory.cpp
)

target_include_directories(configparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(configparser PUBLIC cxx_std_17)