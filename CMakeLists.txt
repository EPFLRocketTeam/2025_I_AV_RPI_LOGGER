cmake_minimum_required(VERSION 3.10)

# Project name and C++ standard
project(rpi_logger VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)       # Use C++17
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add an executable from your source file(s)
add_executable(rpi_logger main.cpp)
