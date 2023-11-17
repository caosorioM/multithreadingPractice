cmake_minimum_required(VERSION 3.20.0)
project(multithreading VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/include")
include(NoInSourceBuilds)

add_subdirectory(src bin)
add_subdirectory(test)
