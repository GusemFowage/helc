cmake_minimum_required(VERSION 3.28)
project(hello)

set(CMAKE_CXX_STANDARD 26)

set(SOURCE_MODULE hello_module)

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