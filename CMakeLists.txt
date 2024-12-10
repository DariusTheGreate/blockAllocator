cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDART 17)

project(BlockAllocator)

set(SOURCES main.cpp BlockAllocator.cpp)

add_executable(BlockAllocator ${SOURCES})
