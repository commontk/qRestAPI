
CONFIGURE_FILE(
  ${qCDashAPI_SOURCE_DIR}/CMake/UseqCDashAPI.cmake.in
  ${qCDashAPI_BINARY_DIR}/UseqCDashAPI.cmake COPYONLY)

# Include directories
SET(qCDashAPI_INCLUDE_DIRS_CONFIG ${qCDashAPI_SOURCE_DIR})

# UseqCDashAPI file
SET(qCDashAPI_USE_FILE_CONFIG ${qCDashAPI_BINARY_DIR}/UseqCDashAPI.cmake)

EXPORT(TARGETS qCDashAPI FILE ${qCDashAPI_BINARY_DIR}/qCDashAPIExports.cmake)

# Configure qCDashAPIConfig.cmake
CONFIGURE_FILE(
  ${qCDashAPI_SOURCE_DIR}/CMake/qCDashAPIConfig.cmake.in
  ${qCDashAPI_BINARY_DIR}/qCDashAPIConfig.cmake @ONLY)
