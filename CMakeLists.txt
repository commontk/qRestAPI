cmake_minimum_required(VERSION 3.16.3...3.19.7 FATAL_ERROR)

project(qRestAPI)

set(CMAKE_INCLUDE_CURRENT_DIR 1)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE 1)
set(CMAKE_POSITION_INDEPENDENT_CODE 1)

# --------------------------------------------------------------------------
# Set directories
# --------------------------------------------------------------------------
#
# CMake
#
set(${PROJECT_NAME}_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
set(CMAKE_MODULE_PATH ${qRestAPI_CMAKE_DIR} ${CMAKE_MODULE_PATH})
#
# Include
#
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
#
# Library
#
set(${PROJECT_NAME}_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} library dirs" FORCE)

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# Qt5
#
set(QT5_INSTALL_PREFIX "" CACHE PATH "The install location of Qt5")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_INSTALL_PREFIX})

set(qRestAPI_QT5_COMPONENTS Core Gui Network Script Test)
find_package(Qt5 COMPONENTS ${qRestAPI_QT5_COMPONENTS} REQUIRED)

# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_TESTING "Test the project" ON)

if(BUILD_SHARED_LIBS)
  set(qRestAPI_STATIC OFF)
else()
  set(qRestAPI_STATIC ON)
endif()

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
configure_file(
  qRestAPI_Export.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/qRestAPI_Export.h
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()

if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)

  if(NOT DEFINED ${PROJECT_NAME}_INSTALL_INCLUDE_DIR)
    set(${PROJECT_NAME}_INSTALL_INCLUDE_DIR include/${PROJECT_NAME})
  endif()

  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers}
    DESTINATION ${${PROJECT_NAME}_INSTALL_INCLUDE_DIR} COMPONENT Development)

  file(GLOB headers "${CMAKE_CURRENT_BINARY_DIR}/*.h")
  install(
    FILES ${headers}
    DESTINATION ${${PROJECT_NAME}_INSTALL_INCLUDE_DIR} COMPONENT Development)
endif()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(KIT_SRCS
  qGirderAPI.cpp
  qGirderAPI.h
  qMidasAPI.cpp
  qMidasAPI.h
  qRestAPI.cpp
  qRestAPI.h
  qRestAPI_p.h
  qRestResult.cpp
  qRestResult.h
  )

set(KIT_MOC_SRCS
  qGirderAPI.h
  qMidasAPI.h
  qRestAPI.h
  qRestAPI_p.h
  qRestResult.h
  )

qt5_wrap_cpp(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
add_library(${PROJECT_NAME} ${KIT_SRCS} ${KIT_MOC_OUTPUT})
target_link_libraries(${PROJECT_NAME} Qt5::Gui Qt5::Network Qt5::Script)


# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Configure support files
# --------------------------------------------------------------------------
# Set qRestAPI_INSTALL_CMAKE_DIR
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_CMAKE_DIR)
  set(${PROJECT_NAME}_INSTALL_CMAKE_DIR cmake/${PROJECT_NAME})
endif()

export(TARGETS qRestAPI FILE ${qRestAPI_BINARY_DIR}/qRestAPIExports.cmake)

# Configure qRestAPIConfig.cmake
configure_file(
  ${qRestAPI_SOURCE_DIR}/qRestAPIConfig.cmake.in
  ${qRestAPI_BINARY_DIR}/qRestAPIConfig.cmake @ONLY)

# Configure qRestAPIConfig.cmake
configure_file(
  ${qRestAPI_SOURCE_DIR}/qRestAPIInstallConfig.cmake.in
  ${qRestAPI_BINARY_DIR}/install/qRestAPIConfig.cmake @ONLY)

# --------------------------------------------------------------------------
# Install configuration files
# --------------------------------------------------------------------------

# Install qRestAPIConfig cmake files
install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/install/qRestAPIConfig.cmake
  DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKE_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  include(CTest)
  add_subdirectory(Testing)
endif()
