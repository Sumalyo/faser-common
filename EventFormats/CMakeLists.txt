if (${CMAKE_PROJECT_NAME} STREQUAL "Calypso")
  # Calypso/Athena build

  message("EventFormats: Calypso")

  atlas_subdir(EventFormats)

  atlas_depends_on_subdirs (
    PUBLIC
    Exceptions
    PRIVATE
    Logging
  )

  atlas_add_library ( EventFormats
  		    EventFormats/*.hpp
		    PUBLIC_HEADERS EventFormats
		    PRIVATE_INCLUDE_DIRS Logging
		    LINK_LIBRARIES Exceptions 
		    PRIVATE_LINK_LIBRARIES Logging
		    )

  # Need this because there are no source files
  set_target_properties(EventFormats PROPERTIES LINKER_LANGUAGE CXX)

  atlas_add_executable( eventDump apps/eventDump.cxx 
      PRIVATE_INCLUDE_DIRS Logging
      LINK_LIBRARIES EventFormats       
    )

else()
  # Online build
  add_library(EventFormats INTERFACE)
  target_include_directories(EventFormats INTERFACE ./)
  target_link_libraries(EventFormats INTERFACE Exceptions Logging)

  add_faser_executable(eventDump apps/eventDump.cxx)
  target_link_libraries(eventDump EventFormats)
endif()
