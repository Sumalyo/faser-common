if (${CMAKE_PROJECT_NAME} STREQUAL "Calypso")
# Calypso/athena build

  message("faser-common build under calypso")
  # Don't do anything here, use subdirectories directly as packages

else()
# Online build

cmake_minimum_required(VERSION 3.4.3)

project(faser_common)

# Require a C++17 compliant compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

macro(add_faser_executable name)
    # Define the executable
    add_executable(${name} ${ARGN})
    if( EXISTS "${CMAKE_SOURCE_DIR}/daqling/src/Utils/Logging.hpp")

    	# Add the required logging
	target_sources(${name} PRIVATE "${CMAKE_SOURCE_DIR}/faser-common/Logging/src/Logger_impl.cpp")
    	target_link_libraries(${name} zmq)
    endif()
endmacro()


add_subdirectory(Logging)
add_subdirectory(Exceptions)
add_subdirectory(EventFormats)
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME))
   add_subdirectory(tests)
endif()


endif()
