cmake_minimum_required(VERSION 3.10)

project(DensityServer VERSION 3.10 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(server_lib
    src/HttpServer.cpp
    src/Request.cpp
    src/Response.cpp
    src/JsonValue.cpp
    src/RequestHandler.cpp
    src/Router.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/utils)

target_include_directories(
    server_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(server server.cpp)
add_executable(test tests/test.cpp)

target_link_libraries(server PRIVATE server_lib)
target_link_libraries(test PRIVATE server_lib)

set(CMAKE_BUILD_TYPE Debug)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
else()
    add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif()