cmake_minimum_required(VERSION 3.14)
project(USystemService LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS WebSockets REQUIRED)

set(HEADERS
    network/HttpClient.h
    network/TcpServer.h
    network/WebsocketClient.h
    service/LocalService.h
    service/RemoteService.h
    tasks/Task.h
    tasks/TaskExecutor.h
    tasks/TaskQueue.h
    )

set(SOURCES
    main.cpp
    network/HttpClient.cpp
    network/TcpServer.cpp
    network/WebsocketClient.cpp
    service/LocalService.cpp
    service/RemoteService.cpp

    tasks/TaskExecutor.cpp
    tasks/TaskQueue.cpp
    )

add_executable(USystemService
    ${HEADERS}
    ${SOURCES}
    )

target_include_directories(USystemService PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/spdlog
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/qtservice
    )

target_link_libraries(USystemService PRIVATE
    Qt5::Core
    Qt5::Network
    Qt5::WebSockets
    qtservice
    spdlog
    ${UNIX_LINK_OPTION}
    )
