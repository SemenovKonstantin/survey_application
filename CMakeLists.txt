cmake_minimum_required(VERSION 3.10)
project(PollingApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Настройка пути к cpp-jwt
include_directories(${CMAKE_SOURCE_DIR}/cpp-jwt/include)
link_directories(${CMAKE_SOURCE_DIR}/cpp-jwt/build)

include_directories(
    ${LIBWEBSOCKETS_INCLUDE_DIRS}
    ${LIBPQXX_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    src/common
)

add_executable(polling_app
    src/backend/main.cpp
    src/backend/server.cpp
    src/backend/database.cpp
)

target_link_libraries(polling_app
    ${LIBWEBSOCKETS_LIBRARIES}
    ${LIBPQXX_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    jwt
)
