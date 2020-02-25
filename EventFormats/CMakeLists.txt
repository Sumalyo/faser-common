add_library(EventFormats INTERFACE)
target_include_directories(EventFormats INTERFACE ./)
target_link_libraries(EventFormats INTERFACE Exceptions)
