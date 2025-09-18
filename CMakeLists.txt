cmake_minimum_required(VERSION 3.10)

project(DensityServer VERSION 3.10 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(server_lib
    src/HttpServer.cpp
    src/Request.cpp
    src/Response.cpp
)

target_include_directories(
    server_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# target_link_directories(server_lib PUBLIC Threads::Threads)

add_executable(server server.cpp)

target_link_libraries(server PRIVATE server_lib)

# install(TARGETS server DESTINATION bin)
# install(DIRECTORY include/ DESTINATION include)

find_package(GTest REQUIRED)
enable_testing()
add_executable(test_program tests/test_gtest.cpp)
target_link_libraries(test_program PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME MyTest COMMAND test_program)