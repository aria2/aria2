find_package(PackageHandleStandardArgs)


FIND_PATH(CPPUNIT_INCLUDE_DIR
	cppunit/TestCase.h
	PATHS
  	cppunit/include
		/usr/local/include
		/usr/include
		${CMAKE_BINARY_DIR}/cppunit-src/include
		${CMAKE_BINARY_DIR}/cppunit-build/include/cppunit
)

# With Win32, important to have both
IF(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  FIND_LIBRARY(CPPUNIT_LIBRARY cppunit
    ${CPPUNIT_INCLUDE_DIR}/../lib
    /usr/local/lib
    /usr/lib
	  ${CMAKE_BINARY_DIR}/cppunit-src/lib/Release
	)
	       
  FIND_LIBRARY(CPPUNIT_DEBUG_LIBRARY cppunitd
    ${CPPUNIT_INCLUDE_DIR}/../lib
    /usr/local/lib
    /usr/lib
	  ${CMAKE_BINARY_DIR}/cppunit-src/lib/Debug
	)
else()
  # On unix system, debug and release have the same name
  FIND_LIBRARY(CPPUNIT_LIBRARY cppunit
    ${CPPUNIT_INCLUDE_DIR}/../lib
    /usr/local/lib
    /usr/lib
	  ${CMAKE_BINARY_DIR}/cppunit-src/lib
	)
  FIND_LIBRARY(CPPUNIT_DEBUG_LIBRARY cppunit
    ${CPPUNIT_INCLUDE_DIR}/../lib
    /usr/local/lib
    /usr/lib
	  ${CMAKE_BINARY_DIR}/cppunit-src/lib
	)
endif()

if(CPPUNIT_INCLUDE_DIR)
  if(CPPUNIT_LIBRARY)
    set(CPPUNIT_LIBRARIES ${CPPUNIT_LIBRARY} ${CMAKE_DL_LIBS})
    set(CPPUNIT_DEBUG_LIBRARIES ${CPPUNIT_DEBUG_LIBRARY} ${CMAKE_DL_LIBS})
  endif()

  set(_version_regex "^#define[ \t]+CPPUNIT_PACKAGE_VERSION[ \t]+\"([^\"]+)\".*")
  file(STRINGS "${CPPUNIT_INCLUDE_DIR}/cppunit/config-auto.h" CPPUNIT_VERSION REGEX "${_version_regex}")
  string(REGEX REPLACE "${_version_regex}" "\\1" CPPUNIT_VERSION ${CPPUNIT_VERSION})
  unset(_version_regex)
endif()



include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(CPPUNIT
  REQUIRED_VARS CPPUNIT_LIBRARIES CPPUNIT_INCLUDE_DIR CPPUNIT_LIBRARY CPPUNIT_DEBUG_LIBRARY
  VERSION_VAR CPPUNIT_VERSION
)
mark_as_advanced(CPPUNIT_INCLUDE_DIR CPPUNIT_LIBRARY CPPUNIT_DEBUG_LIBRARY CPPUNIT_LIBRARIES CPPUNIT_VERSION)

