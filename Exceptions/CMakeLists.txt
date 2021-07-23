if (${CMAKE_PROJECT_NAME} STREQUAL "Calypso")
  # Calypso/Athena build

  message("Exceptions: Calypso")
  atlas_subdir(Exceptions)

  atlas_add_library ( Exceptions
    Exceptions/*.hpp
    PUBLIC_HEADERS Exceptions
  )

  # Need this because there are no source files
  set_target_properties(Exceptions PROPERTIES LINKER_LANGUAGE CXX)
else()
  # Online build
  add_library(Exceptions INTERFACE)
  target_include_directories(Exceptions INTERFACE ./)
  if (${CMAKE_PROJECT_NAME} STREQUAL "daqling_top")
    target_compile_definitions(Exceptions INTERFACE DAQLING_BUILD)
  endif()

endif()
