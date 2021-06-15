if (${CMAKE_PROJECT_NAME} STREQUAL "Calypso")
  # Calypso/Athena build

  message("EventFormats: Calypso")

  atlas_subdir(EventFormats)

  atlas_add_library ( EventFormats
  		  EventFormats/*.hpp EventFormats/*.icc 
        INTERFACE
		    PUBLIC_HEADERS EventFormats 
		    LINK_LIBRARIES Exceptions
		    )
         
  target_include_directories( EventFormats INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Logging/include>
  )


   atlas_add_executable( eventDump apps/eventDump.cxx 
      LINK_LIBRARIES EventFormats
   )

   target_include_directories( eventDump PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Logging/include>
   )

   atlas_add_executable( eventFilter apps/eventFilter.cxx 
      LINK_LIBRARIES EventFormats
   )

   target_include_directories( eventFilter PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Logging/include>
   )
   
else()
  # Online build
  add_library(EventFormats INTERFACE)
  target_include_directories(EventFormats INTERFACE ./)
  target_link_libraries(EventFormats INTERFACE Exceptions Logging )

  add_faser_executable(eventDump apps/eventDump.cxx)
  target_link_libraries(eventDump EventFormats )
  add_faser_executable(eventFilter apps/eventFilter.cxx)
  target_link_libraries(eventFilter EventFormats )

endif()
