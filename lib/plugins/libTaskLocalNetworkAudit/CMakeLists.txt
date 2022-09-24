cmake_minimum_required(VERSION 3.10)
project(libTaskLocalNetworkAudit LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS
    TaskPlugin.h
    TaskItem.h
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

find_package(OpenSSL REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

add_library(TaskLocalNetworkAudit SHARED
    ${SOURCES}
    ${HEADERS}
    )

target_include_directories(TaskLocalNetworkAudit
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../libTask/
    )

if(WIN32)
    target_include_directories(TaskLocalNetworkAudit
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../snmp++_win
        )
endif()

if(UNIX)
    target_include_directories(TaskLocalNetworkAudit
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../snmp++_unix
        )
endif()

target_link_libraries(TaskLocalNetworkAudit
    PUBLIC
    Qt5::Core
    Qt5::Network
    OpenSSL::SSL
    OpenSSL::Crypto
    snmp++
    ${UNIX_LINK_OPTION}
    )

target_compile_definitions(TaskLocalNetworkAudit PRIVATE LIBTOEXPORT)

install(TARGETS TaskLocalNetworkAudit
    RUNTIME DESTINATION lib/plugins/tasks
    LIBRARY DESTINATION lib/plugins/tasks
    )

if (MSVC)
    #install(FILES $<TARGET_PDB_FILE:TaskLocalNetworkAudit> DESTINATION lib/plugins/tasks OPTIONAL)
endif()
