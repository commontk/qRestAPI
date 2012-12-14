
CONFIGURE_FILE(
  ${qRestAPI_SOURCE_DIR}/CMake/UseqRestAPI.cmake.in
  ${qRestAPI_BINARY_DIR}/UseqRestAPI.cmake COPYONLY)

# Include directories
SET(qRestAPI_INCLUDE_DIRS_CONFIG ${qRestAPI_SOURCE_DIR})

# UseqRestAPI file
SET(qRestAPI_USE_FILE_CONFIG ${qRestAPI_BINARY_DIR}/UseqRestAPI.cmake)

EXPORT(TARGETS qRestAPI FILE ${qRestAPI_BINARY_DIR}/qRestAPIExports.cmake)

# Configure qRestAPIConfig.cmake
CONFIGURE_FILE(
  ${qRestAPI_SOURCE_DIR}/CMake/qRestAPIConfig.cmake.in
  ${qRestAPI_BINARY_DIR}/qRestAPIConfig.cmake @ONLY)
