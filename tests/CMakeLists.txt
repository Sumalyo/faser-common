if (${CMAKE_PROJECT_NAME} STREQUAL "Calypso")
else()

add_executable(test_logger test_logger.cpp)
target_link_libraries(test_logger PRIVATE Logging)

add_executable(test_exceptions test_exceptions.cpp)
target_link_libraries(test_exceptions PRIVATE Logging Exceptions)

add_executable(test_DAQFormats test_DAQFormats.cpp)
target_link_libraries(test_DAQFormats PRIVATE EventFormats Logging)

if ("${CMAKE_PROJECT_NAME}" STREQUAL "daqling_top")
  target_link_libraries(test_exceptions PRIVATE ers)
  target_link_libraries(test_DAQFormats PRIVATE ers)
endif()


endif()
