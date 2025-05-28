cmake_minimum_required(VERSION 3.20)

project(tipe)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(tipe ${SOURCES})
