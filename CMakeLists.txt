cmake_minimum_required(VERSION 3.10)

# Project name and C++ standard
project(rpi_logger VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set include directories for the target
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib/2024_C_AV-GS_CAPSULE/src
    ${PROJECT_SOURCE_DIR}/lib/logger
)

# Source files
set(SOURCES
    src/main.cpp
    lib/logger/logger.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})
