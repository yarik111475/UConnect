cmake_minimum_required(VERSION 3.10)
project(libTaskSnmpMonitoring LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS
    TaskItem.h
    TaskPlugin.h
    )
set(SOURCES
    TaskPlugin.cpp
    )

find_package(OpenSSL REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

add_library(TaskSnmpMonitoring SHARED
    ${SOURCES}
    ${HEADERS}
    )

target_include_directories(TaskSnmpMonitoring
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../libTask/
    )

if(WIN32)
    target_include_directories(TaskSnmpMonitoring
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../snmp++_win
        )
endif()

if(UNIX)
    target_include_directories(TaskSnmpMonitoring
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../snmp++_unix
        )
endif()

target_link_libraries(TaskSnmpMonitoring
    PRIVATE
    Qt5::Core
    Qt5::Network
    OpenSSL::SSL
    OpenSSL::Crypto
    spdlog
    snmp++
    ${WIN_LIBS}
    ${UNIX_LIBS}
    )

target_compile_definitions(TaskSnmpMonitoring PRIVATE LIBTOEXPORT)

install(TARGETS TaskSnmpMonitoring
    RUNTIME DESTINATION lib/plugins/tasks
    LIBRARY DESTINATION lib/plugins/tasks
    )

if (MSVC)
    #install(FILES $<TARGET_PDB_FILE:TaskSnmpMonitoring> DESTINATION lib/plugins/tasks OPTIONAL)
endif()
