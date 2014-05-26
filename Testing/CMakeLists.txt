include_directories(
  ${qRestAPI_SOURCE_DIR}
  )

create_test_sourcelist(qRestAPITests_SRC
  qRestAPITests.cpp
  qMidasAPITest.cpp
  )

add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

add_executable(qRestAPITests ${qRestAPITests_SRC})
target_link_libraries(qRestAPITests qRestAPI)
if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  target_link_libraries(qRestAPITests Qt5::Test)
endif()

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND qRestAPITests ${TESTNAME})
endmacro()

SIMPLE_TEST(qMidasAPITest)
