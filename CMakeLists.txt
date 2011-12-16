cmake_minimum_required(VERSION 2.8.2)

PROJECT(qMidasAPI)

OPTION(BUILD_TESTING "Test the project" ON)

# HACK: QtXml is required because of "http://www.richelbilderbeek.nl/CppLinkErrorUndefinedReferenceToQListData.htm"
FIND_PACKAGE(Qt4 4.6.2 COMPONENTS QtCore QtGui QtNetwork QtScript QtXml QtTest REQUIRED)
INCLUDE(${QT_USE_FILE})

SET(KIT_SRCS
  qMidasAPI.cpp
  qMidasAPI.h
  qMidasAPI_p.h
  )

SET(KIT_MOC_SRCS
  qMidasAPI.h
  qMidasAPI_p.h
  )

QT4_WRAP_CPP(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})

ADD_LIBRARY(qMidasAPI ${KIT_SRCS} ${KIT_MOC_OUTPUT})
TARGET_LINK_LIBRARIES(qMidasAPI ${QT_LIBRARIES})

# To fix compilation problem: relocation R_X86_64_32 against `a local symbol' can not be
# used when making a shared object; recompile with -fPIC
IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
ENDIF()

INCLUDE(CTest)
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()

INCLUDE(CMake/GenerateqMidasAPIConfig.cmake)
