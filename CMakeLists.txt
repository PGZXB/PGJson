set(DCMAKE_SH="CMAKE_SH-NOTFOUND")

cmake_minimum_required(VERSION 3.16)
project(PGJson)

add_definitions(-DPGJSON_DEBUG -Wall)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)
add_library(PGJson STATIC src/Allocator.cpp src/Node.cpp src/String.cpp src/utils.cpp include/PGJson/Parser.h)


project(PGTest)
add_executable(PGTest include src/Node.cpp src/main.cpp src/utils.cpp src/String.cpp src/Allocator.cpp)
