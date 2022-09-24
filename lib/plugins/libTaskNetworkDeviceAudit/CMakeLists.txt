cmake_minimum_required(VERSION 3.10)
project(libTaskNetworkDeviceAuditProj LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS
    TaskPlugin.h
    )

if(UNIX)
    set(HEADERS
        ${HEADERS}
        unix/Arp.h
        )
endif()

set(SOURCES
    TaskPlugin.cpp
    )

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

add_library(TaskNetworkDeviceAudit SHARED
    ${SOURCES}
    ${HEADERS}
    )

target_include_directories(TaskNetworkDeviceAudit
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../libTask
    )

target_link_libraries(TaskNetworkDeviceAudit
    PRIVATE
    Qt5::Core
    Qt5::Network
    ${UNIX_LINK_OPTION}
    )

target_compile_definitions(TaskNetworkDeviceAudit PRIVATE LIBTOEXPORT)

install(TARGETS TaskNetworkDeviceAudit
    RUNTIME DESTINATION lib/plugins/tasks
    LIBRARY DESTINATION lib/plugins/tasks
    )

if (MSVC)
    #install(FILES $<TARGET_PDB_FILE:TaskNetworkDeviceAudit> DESTINATION lib/plugins/tasks OPTIONAL)
endif()
