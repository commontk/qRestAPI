cmake_minimum_required(VERSION 3.16.3...3.19.7 FATAL_ERROR)

project(qRestAPI)

set(CMAKE_INCLUDE_CURRENT_DIR 1)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE 1)
set(CMAKE_POSITION_INDEPENDENT_CODE 1)

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_TESTING "Test the project" ON)

set(QT5_INSTALL_PREFIX "" CACHE PATH "The install location of Qt5")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_INSTALL_PREFIX})

set(qRestAPI_QT5_COMPONENTS Core Gui Network Script Test)
find_package(Qt5 COMPONENTS ${qRestAPI_QT5_COMPONENTS} REQUIRED)

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

qt5_wrap_cpp(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})

add_library(${PROJECT_NAME} ${KIT_SRCS} ${KIT_MOC_OUTPUT})
target_link_libraries(${PROJECT_NAME} Qt5::Gui Qt5::Network Qt5::Script)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

include(CMake/GenerateqRestAPIConfig.cmake)
