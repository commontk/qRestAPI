include(MacroAddFileDependencies)

set(KIT_TEST_SRCS
  qGirderAPITest.cpp
  qMidasAPITest.cpp
  qRestAPITest.cpp
  )

create_test_sourcelist(KIT_TESTDRIVER_SRCS qRestAPITests.cpp
  ${KIT_TEST_SRCS}
  )

set(KIT_TEST_GENERATE_MOC_SRCS ${KIT_TEST_SRCS})

foreach(file IN LISTS KIT_TEST_GENERATE_MOC_SRCS)
  get_filename_component(abs_file ${file} ABSOLUTE)
  get_filename_component(source_name ${file} NAME_WE)
  set(moc_file ${CMAKE_CURRENT_BINARY_DIR}/moc_${source_name}.cpp)
  QT5_GENERATE_MOC(${abs_file} ${moc_file})
  macro_add_file_dependencies(${abs_file} ${moc_file})
endforeach()

add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

add_executable(qRestAPITests ${KIT_TESTDRIVER_SRCS})
target_link_libraries(qRestAPITests qRestAPI Qt5::Test)

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND qRestAPITests ${TESTNAME})
endmacro()

SIMPLE_TEST(qGirderAPITest)
SIMPLE_TEST(qMidasAPITest)
SIMPLE_TEST(qRestAPITest)
