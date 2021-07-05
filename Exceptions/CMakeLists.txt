if (${CMAKE_PROJECT_NAME} STREQUAL "Calypso")
  # Calypso/Athena build

  message("Exceptions: Calypso")
  add_compile_definitions(OFFLINE_BUILD)
  atlas_subdir(Exceptions)

  atlas_add_library ( Exceptions
    Exceptions/*.hpp
    PUBLIC_HEADERS Exceptions
  )

  # Need this because there are no source files
  set_target_properties(Exceptions PROPERTIES LINKER_LANGUAGE CXX)
else()
  # Online build
  target_link_libraries(Exceptions PUBLIC ers)
  add_library(Exceptions INTERFACE)
  target_include_directories(Exceptions INTERFACE ./)

endif()
