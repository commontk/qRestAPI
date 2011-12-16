
CONFIGURE_FILE(
  ${qMidasAPI_SOURCE_DIR}/CMake/UseqMidasAPI.cmake.in
  ${qMidasAPI_BINARY_DIR}/UseqMidasAPI.cmake COPYONLY)

# Include directories
SET(qMidasAPI_INCLUDE_DIRS_CONFIG ${qMidasAPI_SOURCE_DIR})

# UseqMidasAPI file
SET(qMidasAPI_USE_FILE_CONFIG ${qMidasAPI_BINARY_DIR}/UseqMidasAPI.cmake)

EXPORT(TARGETS qMidasAPI FILE ${qMidasAPI_BINARY_DIR}/qMidasAPIExports.cmake)

# Configure qMidasAPIConfig.cmake
CONFIGURE_FILE(
  ${qMidasAPI_SOURCE_DIR}/CMake/qMidasAPIConfig.cmake.in
  ${qMidasAPI_BINARY_DIR}/qMidasAPIConfig.cmake @ONLY)
