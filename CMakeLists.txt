cmake_minimum_required(VERSION 3.28)
project(hello)

set(CMAKE_CXX_STANDARD 26)

set(SOURCE_MODULE hello_module)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../app")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../app")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../app")

include_directories(./src)

file(GLOB_RECURSE SRC_IXX "src/*.ixx")

add_library(${SOURCE_MODULE})
target_sources(${SOURCE_MODULE}
        PUBLIC
        FILE_SET CXX_MODULES FILES
        ${SRC_IXX}
)
add_executable(hello main.cpp
        src/attrdef.hxx)
target_link_libraries(hello ${SOURCE_MODULE})