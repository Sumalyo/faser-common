add_library(EventFormats INTERFACE)
target_include_directories(EventFormats INTERFACE ./)
target_link_libraries(EventFormats INTERFACE Exceptions)

add_executable(eventDump apps/eventDump.cxx)
target_link_libraries(eventDump EventFormats)
