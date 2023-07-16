cmake_minimum_required(VERSION 3.10)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9 CACHE STRING "Build for 10.11")

project(tipsy-encoder VERSION 0.9.0 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

add_executable(${PROJECT_NAME}-test test/main.cpp test/binary.cpp)
target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}-test PRIVATE test)
target_compile_definitions(${PROJECT_NAME}-test PRIVATE CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS)


