cmake_minimum_required(VERSION 3.5.0)

project(qRestAPI)

set(CMAKE_INCLUDE_CURRENT_DIR 1)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE 1)
set(CMAKE_POSITION_INDEPENDENT_CODE 1)

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_TESTING "Test the project" ON)

set(qRestAPI_QT_VERSION "4" CACHE STRING "Expected Qt version")
mark_as_advanced(qRestAPI_QT_VERSION)
set_property(CACHE qRestAPI_QT_VERSION PROPERTY STRINGS 4 5)
if(NOT (qRestAPI_QT_VERSION VERSION_EQUAL "4" OR qRestAPI_QT_VERSION VERSION_EQUAL "5"))
  message(FATAL_ERROR "Expected value for qRestAPI_QT_VERSION is either '4' or '5'")
endif()

if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  set(QT5_INSTALL_PREFIX "" CACHE PATH "The install location of Qt5")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_INSTALL_PREFIX})
endif()

if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  set(qRestAPI_QT5_COMPONENTS Core Gui Network Script Test)
  find_package(Qt5 COMPONENTS ${qRestAPI_QT5_COMPONENTS} REQUIRED)
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

if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  qt5_wrap_cpp(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})
else()
  QT4_WRAP_CPP(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})
endif()

add_library(${PROJECT_NAME} ${KIT_SRCS} ${KIT_MOC_OUTPUT})
if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  target_link_libraries(${PROJECT_NAME} Qt5::Gui Qt5::Network Qt5::Script)
else()
  target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

include(CMake/GenerateqRestAPIConfig.cmake)
