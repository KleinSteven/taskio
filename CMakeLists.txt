cmake_minimum_required(VERSION 3.22)

project(taskio VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_library(taskio STATIC
        lib/taskio/taskio.cpp)
add_library(taskio::taskio ALIAS taskio)

target_include_directories(taskio
                            PUBLIC
                            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                            "$<INSTALL_INTERFACE:include>"
)

add_subdirectory(lib)
add_subdirectory(example)
add_subdirectory(test)