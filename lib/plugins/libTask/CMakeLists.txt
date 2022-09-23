cmake_minimum_required(VERSION 3.14)
project(libTask LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

set(HEADERS
    #ICTaskPlugin.h
    ITaskPlugin.h
    ITaskPlugin_global.h
    )
set(SOURCES
    #ITaskPlugin_impl.cpp

    )

add_library(Task STATIC
    ${HEADERS}
    ${SOURCES}
)

target_link_libraries(Task PRIVATE
    Qt5::Core
    )

target_compile_definitions(Task PRIVATE TASK_LIBRARY)
