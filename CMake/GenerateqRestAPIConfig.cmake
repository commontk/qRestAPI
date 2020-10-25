
configure_file(
  ${qRestAPI_SOURCE_DIR}/CMake/UseqRestAPI.cmake.in
  ${qRestAPI_BINARY_DIR}/UseqRestAPI.cmake COPYONLY)

# Include directories
set(qRestAPI_INCLUDE_DIRS_CONFIG ${qRestAPI_INCLUDE_DIRS})

# Library directories
set(qRestAPI_LIBRARY_DIRS_CONFIG ${qRestAPI_BINARY_DIR})

# UseqRestAPI file
set(qRestAPI_USE_FILE_CONFIG ${qRestAPI_BINARY_DIR}/UseqRestAPI.cmake)

export(TARGETS qRestAPI FILE ${qRestAPI_BINARY_DIR}/qRestAPIExports.cmake)

