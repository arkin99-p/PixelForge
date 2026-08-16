cmake_minimum_required(VERSION 3.15)
project(PixelForge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1 -mavx2")

find_package(SDL3 REQUIRED)

add_subdirectory(src/pixelforge)

add_subdirectory(examples)