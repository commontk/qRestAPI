cmake_minimum_required(VERSION 2.8.2)

project(qRestAPI)

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_TESTING "Test the project" ON)

# HACK: QtXml is required because of "http://www.richelbilderbeek.nl/CppLinkErrorUndefinedReferenceToQListData.htm"
find_package(Qt4 4.6.2 COMPONENTS QtCore QtGui QtNetwork QtScript QtXml QtTest REQUIRED)
include(${QT_USE_FILE})

set(qRestAPI_INCLUDE_DIRS ${qRestAPI_SOURCE_DIR} ${qRestAPI_BINARY_DIR})
include_directories(${qRestAPI_INCLUDE_DIRS})

configure_file(
  qRestAPI_Export.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/qRestAPI_Export.h
  )

set(KIT_SRCS
  qMidasAPI.cpp
  qMidasAPI.h
  qMidasAPI_p.h
  qRestAPI.cpp
  qRestAPI.h
  qRestAPI_p.h
  qRestResult.cpp
  qRestResult.h
  qXnatAPI.cpp
  qXnatAPI.h
  qXnatAPI_p.h
  )

set(KIT_MOC_SRCS
  qMidasAPI.h
  qMidasAPI_p.h
  qRestAPI.h
  qRestAPI_p.h
  qRestResult.h
  qXnatAPI.h
  qXnatAPI_p.h
  )

QT4_WRAP_CPP(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})

add_library(${PROJECT_NAME} ${KIT_SRCS} ${KIT_MOC_OUTPUT})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

# To fix compilation problem: relocation R_X86_64_32 against `a local symbol' can not be
# used when making a shared object; recompile with -fPIC
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

include(CMake/GenerateqRestAPIConfig.cmake)

