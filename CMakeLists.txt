cmake_minimum_required(VERSION 2.8.2)

PROJECT(qCDashAPI)

OPTION(BUILD_TESTING "Test the project" ON)

FIND_PACKAGE(Qt4 4.6.2 COMPONENTS QtCore QtGui QtNetwork QtScript REQUIRED)
INCLUDE(${QT_USE_FILE})

SET(KIT_SRCS
  qCDashAPI.cpp
  qCDashAPI.h
  qCDashAPI_p.h
  )

SET(KIT_MOC_SRCS
  qCDashAPI.h
  qCDashAPI_p.h
  )

QT4_WRAP_CPP(KIT_MOC_OUTPUT ${KIT_MOC_SRCS})

ADD_LIBRARY(qCDashAPI ${KIT_SRCS} ${KIT_MOC_OUTPUT})
TARGET_LINK_LIBRARIES(qCDashAPI ${QT_LIBRARIES})

# To fix compilation problem: relocation R_X86_64_32 against `a local symbol' can not be
# used when making a shared object; recompile with -fPIC
IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
ENDIF()

IF(BUILD_TESTING)
  INCLUDE(CTest)
  CREATE_TEST_SOURCELIST(Tests qCDashAPICppTests.cpp
    qCDashAPITest.cpp
    )
  SET(TestsToRun ${Tests})
  REMOVE(TestsToRun qCDashAPICppTests.cpp)

  ADD_EXECUTABLE(qCDashAPICppTests ${Tests})
  TARGET_LINK_LIBRARIES(qCDashAPICppTests qCDashAPI)

  MACRO(QUERY_TEST QUERYNAME)
    ADD_TEST(NAME qCDashAPITest_${QUERYNAME} COMMAND qCDashAPICppTests qCDashAPITest ${QUERYNAME})
  ENDMACRO()

  QUERY_TEST(ProjectFiles)
  SET_PROPERTY(TEST qCDashAPITest_ProjectFiles PROPERTY PASS_REGULAR_EXPRESSION "projectFiles length")

  QUERY_TEST(ProjectList)
  SET_PROPERTY(TEST qCDashAPITest_ProjectList PROPERTY PASS_REGULAR_EXPRESSION "projectList length")

ENDIF()

INCLUDE(CMake/GenerateqCDashAPIConfig.cmake)
