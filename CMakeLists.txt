cmake_minimum_required(VERSION 3.0)
project(replacetag)

option(FP_EXAMPLE "example" off)

set(FP_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src)