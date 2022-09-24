cmake_minimum_required(VERSION 3.10)
project(libTaskCheckAlive LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS
    TaskPlugin.h
    )

set(SOURCES
    TaskPlugin.cpp
    )

find_package(Qt5 COMPONENTS Core REQUIRED)

add_library(TaskCheckAlive SHARED
    ${SOURCES}
    ${HEADERS}
    )

target_include_directories(TaskCheckAlive
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../libTask/
    )

target_link_libraries(TaskCheckAlive
    PUBLIC
    Qt5::Core
    ${UNIX_LINK_OPTION}
    )

target_compile_definitions(TaskCheckAlive PRIVATE LIBTOEXPORT)

install(TARGETS TaskCheckAlive
    RUNTIME DESTINATION lib/plugins/tasks
    LIBRARY DESTINATION lib/plugins/tasks
    )

if (MSVC)
    #install(FILES $<TARGET_PDB_FILE:TaskCheckAlive> DESTINATION lib/plugins/tasks OPTIONAL)
endif()
