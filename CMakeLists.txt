cmake_minimum_required(VERSION 3.10)

project(rpi_logger VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source and header files
set(SOURCES
    src/main.cpp
    lib/logger/logger.cpp
)

set(HEADERS
    include/packet_definition.h
    lib/logger/logger.h
    lib/2024_C_AV-GS_CAPSULE/src/capsule.h
)

# Create executable (sources + headers for IDE visibility)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add include directories (scoped to the target)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib/2024_C_AV-GS_CAPSULE/src
    ${PROJECT_SOURCE_DIR}/lib/logger
)
