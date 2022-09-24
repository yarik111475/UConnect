cmake_minimum_required(VERSION 3.14)

project(UConnect LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(OPENSSL_ROOT_DIR "C:/Program Files (x86)/OpenSSL-Win32")
    endif()
endif()

message("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (UNIX_LINK_OPTION
           -static-libgcc
           -static-libstdc++
           -pthread
        )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

add_subdirectory(lib/spdlog)
add_subdirectory(lib/plugins)
add_subdirectory(lib/qtservice)
add_subdirectory(USystemAgent)
add_subdirectory(USystemService)
add_subdirectory(PluginTester)

if(WIN32)
    add_subdirectory(lib/snmp++_win)
endif()
if(UNIX)
    add_subdirectory(lib/snmp++_unix)
endif()
