include_directories(
  ${qRestAPI_SOURCE_DIR}
  )

create_test_sourcelist(qRestAPITests_SRC
  qRestAPITests.cpp
  qMidasAPITest.cpp
  qXnatAPITest.cpp
  )

add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

set(qXnatAPITest_MOC_SRCS
  qXnatAPITest.h
  )

QT4_WRAP_CPP(qXnatAPITest_MOC_OUTPUT ${qXnatAPITest_MOC_SRCS})

add_executable(qRestAPITests ${qRestAPITests_SRC} ${qXnatAPITest_MOC_OUTPUT})
target_link_libraries(qRestAPITests qRestAPI)

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND qRestAPITests ${TESTNAME})
endmacro()

SIMPLE_TEST(qMidasAPITest)
SIMPLE_TEST(qXnatAPITest)
