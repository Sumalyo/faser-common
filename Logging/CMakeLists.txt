if (${CMAKE_PROJECT_NAME} STREQUAL "Calypso")
  # Calypso/Athena build
  # Do nothing, we will just include the Logging include in EventFormats path

  message("Logging: Calypso")


else()
  # Online build

  add_library(Logging INTERFACE)

  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/daqling/src/Utils/Ers.hpp")
    message(STATUS "No Daqling logging available")
    target_include_directories(Logging INTERFACE include/)
  else()	 
   message(STATUS "With daqling logging")
   target_include_directories(Logging INTERFACE ${CMAKE_SOURCE_DIR}/daqling/src/Utils/)
   target_include_directories(Logging INTERFACE include/)	
  endif()

endif()
