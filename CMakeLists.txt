cmake_minimum_required(VERSION 3.16.3)

project(qRestAPI)

# --------------------------------------------------------------------------
# CMake variables
# --------------------------------------------------------------------------
set(CMAKE_INCLUDE_CURRENT_DIR 1)
set(CMAKE_POSITION_INDEPENDENT_CODE 1)

# --------------------------------------------------------------------------
# Directories
# --------------------------------------------------------------------------
#
# CMake: holds the package-config templates (*Config.cmake.in).
#
set(${PROJECT_NAME}_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
#
# Include
#
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
#
# Library
#
set(${PROJECT_NAME}_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} library dirs" FORCE)

# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_TESTING "Test the project" ON)

# --------------------------------------------------------------------------
# Install directories
# --------------------------------------------------------------------------
# These may be set by a parent project (e.g. a superbuild) before this project
# is added. When not set, sensible standalone defaults are used.
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_INCLUDE_DIR)
  set(${PROJECT_NAME}_INSTALL_INCLUDE_DIR include/${PROJECT_NAME})
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_CMAKE_DIR)
  set(${PROJECT_NAME}_INSTALL_CMAKE_DIR cmake/${PROJECT_NAME})
endif()

# Install the development files (headers, CMake package config, exported
# targets) in addition to the runtime library. OFF by default so a plain
# install ships only the runtime; turn ON for packaging / standalone install
# trees. (CMP0077 is NEW at this cmake_minimum_required, so a parent project may
# still override this before add_subdirectory().)
option(${PROJECT_NAME}_INSTALL_DEVELOPMENT
  "Install ${PROJECT_NAME} development files (headers, CMake package config, exported targets)" OFF)

# Backward compatibility: honor the deprecated, inverted
# ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT if a consumer still sets it.
if(DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  message(DEPRECATION
    "${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT is deprecated; use ${PROJECT_NAME}_INSTALL_DEVELOPMENT (inverted value).")
  if(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
    set(${PROJECT_NAME}_INSTALL_DEVELOPMENT OFF CACHE BOOL "" FORCE)
  else()
    set(${PROJECT_NAME}_INSTALL_DEVELOPMENT ON CACHE BOOL "" FORCE)
  endif()
endif()

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# Qt
#
set(${PROJECT_NAME}_QT_VERSION "5" CACHE STRING "Expected Qt version (5 or 6)")
mark_as_advanced(${PROJECT_NAME}_QT_VERSION)
set_property(CACHE ${PROJECT_NAME}_QT_VERSION PROPERTY STRINGS 5 6)
if(NOT (${PROJECT_NAME}_QT_VERSION VERSION_EQUAL "5" OR ${PROJECT_NAME}_QT_VERSION VERSION_EQUAL "6"))
  message(FATAL_ERROR "Expected value for ${PROJECT_NAME}_QT_VERSION is either '5' or '6'")
endif()

set(QT${${PROJECT_NAME}_QT_VERSION}_INSTALL_PREFIX "" CACHE PATH "The install location of Qt${${PROJECT_NAME}_QT_VERSION}")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT${${PROJECT_NAME}_QT_VERSION}_INSTALL_PREFIX})

# Qt components used through the public interface of the library. They are
# recorded here so the generated package config can re-resolve them through
# find_dependency() for downstream consumers. The Test component is required
# only when building the tests (see Testing/CMakeLists.txt).
set(${PROJECT_NAME}_QT_COMPONENTS Gui Network Qml)
find_package(Qt${${PROJECT_NAME}_QT_VERSION} COMPONENTS Core ${${PROJECT_NAME}_QT_COMPONENTS} REQUIRED)

if(BUILD_SHARED_LIBS)
  set(${PROJECT_NAME}_STATIC OFF)
else()
  set(${PROJECT_NAME}_STATIC ON)
endif()

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
configure_file(
  qRestAPI_Export.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/qRestAPI_Export.h
  )

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

if(${PROJECT_NAME}_QT_VERSION VERSION_GREATER "5")
  qt_wrap_cpp(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})
else()
  qt5_wrap_cpp(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})
endif()

# --------------------------------------------------------------------------
# Build qRestAPI library
# --------------------------------------------------------------------------
add_library(${PROJECT_NAME} ${KIT_SRCS} ${KIT_MOC_OUTPUT})

# Public usage requirements: consumers get the include directories through the
# imported target, resolved differently for the build tree and the install tree.
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${${PROJECT_NAME}_INSTALL_INCLUDE_DIR}>
  )

target_link_libraries(${PROJECT_NAME} PUBLIC
  Qt${${PROJECT_NAME}_QT_VERSION}::Gui
  Qt${${PROJECT_NAME}_QT_VERSION}::Network
  Qt${${PROJECT_NAME}_QT_VERSION}::Qml)

# --------------------------------------------------------------------------
# Export targets for a build tree
# --------------------------------------------------------------------------
export(TARGETS ${PROJECT_NAME}
  FILE ${CMAKE_CURRENT_BINARY_DIR}/qRestAPITargets.cmake)

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME}
  EXPORT qRestAPITargets
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Configure package config file for the build tree
# --------------------------------------------------------------------------
configure_file(
  ${${PROJECT_NAME}_CMAKE_DIR}/qRestAPIConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/qRestAPIConfig.cmake @ONLY)

# --------------------------------------------------------------------------
# Install development files
# --------------------------------------------------------------------------
if(${PROJECT_NAME}_INSTALL_DEVELOPMENT)

  # Public headers (exclude private *_p.h headers)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  list(FILTER headers EXCLUDE REGEX "_p\\.h$")
  install(
    FILES ${headers}
    DESTINATION ${${PROJECT_NAME}_INSTALL_INCLUDE_DIR} COMPONENT Development)

  file(GLOB headers "${CMAKE_CURRENT_BINARY_DIR}/*.h")
  install(
    FILES ${headers}
    DESTINATION ${${PROJECT_NAME}_INSTALL_INCLUDE_DIR} COMPONENT Development)

  # Relocatable package config file for the install tree
  include(CMakePackageConfigHelpers)
  configure_package_config_file(
    ${${PROJECT_NAME}_CMAKE_DIR}/qRestAPIInstallConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/install/qRestAPIConfig.cmake
    INSTALL_DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKE_DIR}
    PATH_VARS ${PROJECT_NAME}_INSTALL_INCLUDE_DIR ${PROJECT_NAME}_INSTALL_LIB_DIR
    )

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/install/qRestAPIConfig.cmake
    DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKE_DIR} COMPONENT Development)

  # Exported targets for the install tree
  install(
    EXPORT qRestAPITargets
    FILE qRestAPITargets.cmake
    DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKE_DIR} COMPONENT Development)

endif()

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  include(CTest)
  add_subdirectory(Testing)
endif()
