cmake_minimum_required(VERSION 3.10)
project(libTaskDisplayDeviceAudit LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(WIN_API_LINK_OPTION
        netapi32
        wbemuuid
        )
endif()

set(HEADERS
    TaskPlugin.h
    win/Wmi.h
    win/WinUtils.h
    )
set(SOURCES
    TaskPlugin.cpp
    win/Wmi.cpp
    win/WinUtils.cpp
    )

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_library(TaskDisplayDeviceAudit SHARED
    ${SOURCES}
    ${HEADERS} )

target_include_directories(TaskDisplayDeviceAudit
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../libTask
    )

target_link_libraries(TaskDisplayDeviceAudit
    PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    spdlog
    ${WIN_API_LINK_OPTION}
    ${UNIX_LINK_OPTION}
    )

target_compile_definitions(TaskDisplayDeviceAudit PRIVATE LIBTOEXPORT)

install(TARGETS TaskDisplayDeviceAudit
    RUNTIME DESTINATION lib/plugins/tasks
    LIBRARY DESTINATION lib/plugins/tasks
    )
