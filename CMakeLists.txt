set(DCMAKE_SH="CMAKE_SH-NOTFOUND")

cmake_minimum_required(VERSION 3.16)
project(PGJson)

add_definitions(-DPGJSON_DEBUG)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)
add_library(PGJson include/PGJson/fwd.h include/PGJson/Node.h src/Node.cpp include/PGJson/Allocator.h include/PGJson/MemoryBlockPool.h include/PGJson/utils.h include/PGJson/Iterator.h include/PGJson/String.h include/PGJson/FileStream.h)

project(PGTest)
add_executable(PGTest include src/Node.cpp src/main.cpp src/utils.cpp src/String.cpp src/Allocator.cpp src/FILEWrapper.cpp)
