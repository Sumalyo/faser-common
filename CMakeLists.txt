cmake_minimum_required(VERSION 3.4.3)

project(faser_common)

# Require a C++17 compliant compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(EventFormats)
add_subdirectory(Exceptions)
add_subdirectory(Logging)
