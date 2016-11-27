cmake_minimum_required(VERSION 3.7)
project(QAmiensRCode)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_definitions(-std=c++11)
add_executable(QAmiensRCode ${SOURCE_FILES})