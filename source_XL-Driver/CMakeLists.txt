# Version of CMake
cmake_minimum_required(VERSION 3.8.2)

# Project name
project(Virtual_Type_4 VERSION 1.0)

# Set build type to Debug (if required)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)

# Define compiler flags for MinGW (optional)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

# Add preprocessor definitions


# Include directories (for header files)
include_directories(${CMAKE_SOURCE_DIR}/include)
#include_directories(${CMAKE_SOURCE_DIR}/lib)

# Gather all .c source files from the src directory
#file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
#file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/customCANdemo.c)
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/xlSPICAN.c)
# file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/xlUartCanDemo.c)

# Find Secur32 library
#find_library(SECUR32_LIB Secur32 REQUIRED)
#if(NOT SECUR32_LIB)
#    message(FATAL_ERROR "Secur32 library not found!")
#endif()
#target_link_libraries(autosar_executable PRIVATE Threads::Threads secur32 vxlapi64)



# Find Pthread library
find_package(Threads REQUIRED)

# Add the shared library (DLL) for vxlapi
add_library(vxlapi64 SHARED IMPORTED)
set_property(TARGET vxlapi64 PROPERTY IMPORTED_LOCATION "C:/toolbase/mingw/comp_10.2.0_w64_1f/bin/vxlapi64.dll")
set_property(TARGET vxlapi64 PROPERTY IMPORTED_IMPLIB "C:/toolbase/mingw/comp_10.2.0_w64_1f/bin/vxlapi64.lib")

# Create the executable
add_executable(autosar_executable ${SOURCES})

# # Link the Winsock library (required for socket functions)
# target_link_libraries(autosar_executable )


# Link libraries to the executable
#target_link_libraries(autosar_executable PRIVATE Threads::Threads ${SECUR32_LIB} vxlapi64)
target_link_libraries(autosar_executable PRIVATE Threads::Threads  vxlapi64 ws2_32)
# Add compile options (optional: adjust flags as needed)
target_compile_options(autosar_executable PRIVATE -Wall -Wextra -Wpedantic)



# Print messages for debugging CMake configuration (optional)
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Secur32 library: ${SECUR32_LIB}")

#### RUN

# mkdir build 
# cd build
# cmake -G "MinGW Makefiles" ..
# make