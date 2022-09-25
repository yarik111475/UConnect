cmake_minimum_required(VERSION 3.14)

project(PluginTester LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SOURCES
    main.cpp
    )

add_executable(PluginTester
    WIN32 ${SOURCES}
    )

target_include_directories(PluginTester PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/plugins//libTask
    )

target_link_libraries(PluginTester PRIVATE
    Qt5::Core
    Qt5::Widgets
    ${UNIX_LIBS}
    )

