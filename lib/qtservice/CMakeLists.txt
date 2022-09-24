cmake_minimum_required(VERSION 3.14)
project(qtservice LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS
    qtservice.h
    qtservice_p.h
    )

set(SOURCES
    qtservice.cpp
    )

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

if(WIN32)
    set(SOURCES
        ${SOURCES}
        qtservice_win.cpp
        )
endif()

if(UNIX)
    set(SOURCES ${SOURCES}
        qtunixsocket.cpp
        qtservice_unix.cpp
        qtunixserversocket.cpp
        )
    set(HEADERS ${HEADERS}
        qtunixsocket.h
        qtunixserversocket.h
        )
endif()

add_library(qtservice STATIC
    ${HEADERS}
    ${SOURCES}
    )

target_link_libraries(qtservice PUBLIC
    Qt5::Core
    Qt5::Network
    )

target_compile_definitions(qtservice PRIVATE QTSERVICE_LIBRARY)
