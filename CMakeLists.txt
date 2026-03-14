cmake_minimum_required(VERSION 3.30)

# Setup project name, version and programming language
# LANGUAGES CXX means C++
project(NGINDX VERSION 0.1.0 LANGUAGES CXX)

# Visual Studio will be using folders and put projects into it
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Create folder path variables
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(TESTS_DIR "${CMAKE_SOURCE_DIR}/tests")
set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor")

# Add sub directories which contains CMakeLists.txt
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(vendor)

# Make sandbox as a startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Sandbox)
