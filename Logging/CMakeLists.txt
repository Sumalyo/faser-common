add_library(Logging INTERFACE)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/daqling/src/Utils/Logging.hpp")
    message(STATUS "No Daqling logging available")
    target_include_directories(Logging INTERFACE include/)
 else()	 
   message(STATUS "With daqling logging")
   target_include_directories(Logging INTERFACE ${CMAKE_SOURCE_DIR}/daqling/src/Utils/)	
endif()
