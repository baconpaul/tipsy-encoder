cmake_minimum_required(VERSION 3.10)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9 CACHE STRING "Build for 10.11")


project(tipsy-encoder VERSION 0.9.0 LANGUAGES CXX)

option(TIPSY_USE_CXX_11 "Use C++ 11 vs 17" OFF)

set(CMAKE_CXX_EXTENSIONS OFF)
if (TIPSY_USE_CXX_11)
    message(STATUS "Setting C++ 11 standard")
    set(CMAKE_CXX_STANDARD 11)
else()
    message(STATUS "Setting C++ 17 standard")
    set(CMAKE_CXX_STANDARD 17)
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

add_executable(${PROJECT_NAME}-test test/main.cpp test/binary.cpp test/protocol.cpp)
target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}-test PRIVATE test)

# This is required for macos < 10.12 which we support here but its only in the test
target_compile_definitions(${PROJECT_NAME}-test PRIVATE CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${PROJECT_NAME}-test PRIVATE -Werror)
endif()


add_custom_target(tipsy-code-checks)

# Clang Format checks
find_program(CLANG_FORMAT_EXE NAMES clang-format-12 clang-format)
set(CLANG_FORMAT_DIRS test include)
set(CLANG_FORMAT_EXTS cpp h)
foreach(dir ${CLANG_FORMAT_DIRS})
    foreach(ext ${CLANG_FORMAT_EXTS})
        list(APPEND CLANG_FORMAT_GLOBS "':(glob)${dir}/**/*.${ext}'")
    endforeach()
endforeach()
add_custom_command(TARGET tipsy-code-checks
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo About to check clang-format using ${CLANG_FORMAT_EXE}
        COMMAND git ls-files -- ${CLANG_FORMAT_GLOBS} | xargs ${CLANG_FORMAT_EXE} --dry-run --Werror
        )
# }}}

