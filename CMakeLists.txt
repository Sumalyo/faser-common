# Seems to be unset in CI
if(NOT DEFINED CMAKE_PROJECT_NAME)
  set(CMAKE_PROJECT_NAME "Unknown")
endif()

if ("Calypso" STREQUAL ${CMAKE_PROJECT_NAME})
# Calypso/athena build

  message("faser-common build under calypso")
  # Don't do anything here, use subdirectories directly as packages

else()
# Online build

cmake_minimum_required(VERSION 3.4.3)

project(faser_common)

option(BE_PEDANTIC "Compile pedantically" ON)

# Require a C++17 compliant compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Set standard build flags
if (BE_PEDANTIC)
  message("enabling a range of pedantic compiler flags to meet daqling standards")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wzero-as-null-pointer-constant -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Wredundant-decls -Wsign-conversion -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Werror -Wformat-security -fdiagnostics-color=auto -Wno-overloaded-virtual")
endif()

macro(add_faser_executable name)
    # Define the executable
    add_executable(${name} ${ARGN})
    if( EXISTS "${CMAKE_SOURCE_DIR}/daqling/src/Utils/Ers.hpp")
    	# Add the required logging
	# target_sources(${name} PRIVATE "${CMAKE_SOURCE_DIR}/faser-common/Logging/src/Logger_impl.cpp")
    	# target_link_libraries(${name} zmq)
    endif()
endmacro()

add_subdirectory(Logging)
add_subdirectory(Exceptions)
add_subdirectory(EventFormats)
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME))
   add_subdirectory(tests)
endif()

endif()
