cmake_minimum_required(VERSION 3.5.0)

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
# CMake
#
set(${PROJECT_NAME}_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
set(CMAKE_MODULE_PATH ${qRestAPI_CMAKE_DIR} ${CMAKE_MODULE_PATH})
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

# By default only runtime files are installed. Set to OFF to also install the
# development files (headers, CMake package config, exported targets), e.g. when
# creating a package or a standalone install tree.
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# Qt
#
set(qRestAPI_QT_VERSION "4" CACHE STRING "Expected Qt version")
mark_as_advanced(qRestAPI_QT_VERSION)
set_property(CACHE qRestAPI_QT_VERSION PROPERTY STRINGS 4 5 6)
if(NOT (qRestAPI_QT_VERSION VERSION_EQUAL "4" OR qRestAPI_QT_VERSION VERSION_EQUAL "5" OR qRestAPI_QT_VERSION VERSION_EQUAL "6"))
  message(FATAL_ERROR "Expected value for qRestAPI_QT_VERSION is either '4', '5' or '6'")
endif()

if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  set(QT${qRestAPI_QT_VERSION}_INSTALL_PREFIX "" CACHE PATH "The install location of Qt${qRestAPI_QT_VERSION}")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT${qRestAPI_QT_VERSION}_INSTALL_PREFIX})
endif()

# Qt components used through the public interface of the library. They are
# recorded here so the generated package config can re-resolve them through
# find_dependency() for downstream consumers.
set(qRestAPI_QT_COMPONENTS Gui Network Qml)

if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  find_package(Qt${qRestAPI_QT_VERSION} COMPONENTS Core ${qRestAPI_QT_COMPONENTS} Test REQUIRED)
else()
  # HACK: QtXml is required because of "http://www.richelbilderbeek.nl/CppLinkErrorUndefinedReferenceToQListData.htm"
  find_package(Qt4 4.6.2 COMPONENTS QtCore QtGui QtNetwork QtScript QtXml QtTest REQUIRED)
  include(${QT_USE_FILE})
endif()

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

if(qRestAPI_QT_VERSION VERSION_GREATER "5")
  qt_wrap_cpp(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})
elseif(qRestAPI_QT_VERSION VERSION_GREATER "4")
  qt5_wrap_cpp(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})
else()
  QT4_WRAP_CPP(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})
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

if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${qRestAPI_QT_VERSION}::Gui
    Qt${qRestAPI_QT_VERSION}::Network
    Qt${qRestAPI_QT_VERSION}::Qml)
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC ${QT_LIBRARIES})
endif()

# --------------------------------------------------------------------------
# Export targets for a build tree
# --------------------------------------------------------------------------
export(TARGETS ${PROJECT_NAME}
  FILE ${qRestAPI_BINARY_DIR}/qRestAPITargets.cmake)

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
  ${qRestAPI_SOURCE_DIR}/qRestAPIConfig.cmake.in
  ${qRestAPI_BINARY_DIR}/qRestAPIConfig.cmake @ONLY)

# --------------------------------------------------------------------------
# Install development files
# --------------------------------------------------------------------------
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)

  # Headers
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
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
    ${qRestAPI_SOURCE_DIR}/qRestAPIInstallConfig.cmake.in
    ${qRestAPI_BINARY_DIR}/install/qRestAPIConfig.cmake
    INSTALL_DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKE_DIR}
    PATH_VARS ${PROJECT_NAME}_INSTALL_INCLUDE_DIR ${PROJECT_NAME}_INSTALL_LIB_DIR
    )

  install(
    FILES ${qRestAPI_BINARY_DIR}/install/qRestAPIConfig.cmake
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
