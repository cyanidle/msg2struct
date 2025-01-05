cmake_minimum_required(VERSION 3.16)

project(msg2struct)

add_executable(test test.cpp)
set_target_properties(test PROPERTIES CXX_STANDARD 11)