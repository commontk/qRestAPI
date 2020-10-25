set(qRestAPI_QT_VERSION "4" CACHE STRING "Expected Qt version")
mark_as_advanced(qRestAPI_QT_VERSION)

set_property(CACHE qRestAPI_QT_VERSION PROPERTY STRINGS 4 5)

if(NOT (qRestAPI_QT_VERSION VERSION_EQUAL "4" OR qRestAPI_QT_VERSION VERSION_EQUAL "5"))
  message(FATAL_ERROR "Expected value for qRestAPI_QT_VERSION is either '4' or '5'")
endif()

if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  cmake_minimum_required(VERSION 2.8.12)
  set(QT5_INSTALL_PREFIX "" CACHE PATH "The install location of Qt5")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_INSTALL_PREFIX})
else()
  cmake_minimum_required(VERSION 2.8.2)
endif()

cmake_policy(SET CMP0048 NEW)
project(qRestAPI VERSION 0.1 )

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_TESTING "Test the project" ON)

if(qRestAPI_QT_VERSION VERSION_GREATER "4")
  set(qRestAPI_QT5_COMPONENTS Core Gui Network Script Test)
  find_package(Qt5 COMPONENTS ${qRestAPI_QT5_COMPONENTS} REQUIRED)
else()
  # HACK: QtXml is required because of "http://www.richelbilderbeek.nl/CppLinkErrorUndefinedReferenceToQListData.htm"
  find_package(Qt4 4.6.2 COMPONENTS QtCore QtGui QtNetwork QtScript QtXml QtTest REQUIRED)
  include(${QT_USE_FILE})
endif()


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
  )

set(KIT_MOC_SRCS
  qMidasAPI.h
  qMidasAPI_p.h
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
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)


# To fix compilation problem: relocation R_X86_64_32 against `a local symbol' can not be
# used when making a shared object; recompile with -fPIC
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
# Installation (https://github.com/forexample/package-example)
include( GNUInstallDirs )
set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/UseqRestAPI.cmake DESTINATION ${config_install_dir} )

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(version_config "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# Include module with function 'write_basic_package_version_file'
include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Use:
#   * PROJECT_VERSION
write_basic_package_version_file(
  "${version_config}" COMPATIBILITY SameMajorVersion
)

# Configure '<PROJECT-NAME>Config.cmake'
# Use variables:
#   * targets_export_name
#   * PROJECT_NAME
configure_package_config_file(
  "CMake/qRestAPIConfig.cmake.in"
  "${project_config}"
  INSTALL_DESTINATION "${config_install_dir}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${targets_export_name}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install( FILES ${KIT_MOC_SRCS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME} )

include(CMake/GenerateqRestAPIConfig.cmake)

