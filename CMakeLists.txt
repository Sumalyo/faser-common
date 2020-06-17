message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
message("PROJECT_NAME = ${PROJECT_NAME}")

if (${CMAKE_PROJECT_NAME} STREQUAL "Calypso")

  message("faser-common")
  # Don't do anything here, use subdirectories directly as packages

endif()
