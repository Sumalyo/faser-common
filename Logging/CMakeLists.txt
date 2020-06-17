if (${CMAKE_PROJECT_NAME} STREQUAL "Calypso")
  # Calypso/Athena build

  message("Logging: Calypso")

  atlas_subdir(Logging)

  atlas_add_library ( Logging
    Logging/*.hpp
    PUBLIC_HEADERS Logging
  )

  # Need this because there are no source files
  set_target_properties(Logging PROPERTIES LINKER_LANGUAGE CXX)

else()
  # Online build

  add_library(Logging INTERFACE)

  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/daqling/src/Utils/Logging.hpp")
    message(STATUS "No Daqling logging available")
    target_include_directories(Logging INTERFACE include/)
  else()	 
   message(STATUS "With daqling logging")
   target_include_directories(Logging INTERFACE ${CMAKE_SOURCE_DIR}/daqling/src/Utils/)	
  endif()

endif()
