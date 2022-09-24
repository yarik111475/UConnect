cmake_minimum_required(VERSION 3.14)
project(USystemAgent LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SOURCES
    main.cpp
    Bootstrap.cpp
    crypto/X509Csr.cpp
    network/HttpClientBase.cpp
    network/HttpClientCert.cpp
    network/HttpClientTask.cpp
    service/LocalService.cpp
    task/TaskExecutor.cpp
    task/TaskQueue.cpp
    utils/Utils.cpp
    )

set(HEADERS
    Bootstrap.h
    crypto/X509Csr.h
    network/HttpClientBase.h
    network/HttpClientCert.h
    network/HttpClientTask.h
    service/LocalService.h
    task/Task.h
    task/TaskExecutor.h
    task/TaskQueue.h
    utils/Utils.h
    )

add_executable(USystemAgent
    ${HEADERS}
    ${SOURCES}
    )

target_include_directories(USystemAgent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/plugins/libTask
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/spdlog
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/qtservice
    )

target_link_libraries(USystemAgent PRIVATE
    Qt5::Core
    Qt5::Network
    Qt5::Widgets
    OpenSSL::SSL
    OpenSSL::Crypto
    qtservice
    spdlog
    ${UNIX_LINK_OPTION}
    )
