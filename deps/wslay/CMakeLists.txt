cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(wslay VERSION 1.1.1 LANGUAGES CXX C)

# list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
message("The CMAKE_MODULE_PATH has the value of ${CMAKE_MODULE_PATH}")

include(Helpers)

include(FindPkgConfig)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckFunctionExists)
include(CheckIncludeFiles)

find_program(SPHINX_BUILD sphinx-build)

if(SPHINX_BUILD)
    set(HAVE_SPHINX_BUILD TRUE)
    message("The sphinx-build was found at path ${SPHINX_BUILD}")
else()
    message(WARNING "The sphinx-build program was not found")
endif()

set(have_nettle FALSE)
pkg_check_modules(NETTLE "nettle >= 2.4")
if(NETTLE_FOUND)
	message(STATUS "Found the NETTLE again")
    set(have_nettle TRUE)
endif()

pkg_check_modules(CUNIT "cunit")

string(APPEND CMAKE_REQUIRED_LIBRARIES " ${CUNIT_LIBRARIES}")
check_cxx_symbol_exists(CU_initialize_registry "TestDB.h" have_cunit)

if(NOT have_cunit)
    message(WARNING "The cunit was not found. Checking for only the definition without declaration")
	check_function_exists(CU_initialize_registry HAVE_CU)
	if(HAVE_CU)
		set(HAVE_CUNIT TRUE)
		set(have_cunit TRUE)  # we could use the HAVE_CUNIT, but for sanity and clearness purposes, update the appropriate variable
	else()
		set(HAVE_CUNIT FALSE)
		message(WARNING "CUnit was not found. Install it with: sudo apt-get install libcunit1-dev")
	endif()
endif()

if(CMAKE_HOST_NAME STREQUAL "MINGW")  # TODOm: should chagne the check of MINGW
	string(APPEND LIBS " -lws2_32")
endif()

check_include_files("arpa/inet.h;netinet/in.h;stddef.h;stdint.h;stdlib.h;string.h;unistd.h;" CHECK_HEADERS)

if(NOT CHECK_HEADERS)
	message(WARNING "One of the headers was not found")
else()
	add_compile_definitions(HAVE_STDINT_H=1)
endif()

if(CMAKE_HOST_NAME STREQUAL "MINGW") # TODOm: should chagne the check of MINGW
	check_include_file("winsock2.h" WINSOCK_HEADER_FILE)

	if(NOT WINSOCK_HEADER_FILE)
		message(WARNING "The winsock2.h header was not found")
	endif()
endif()

check_cxx_symbol_exists(memmove "string.h" HAVE_MEMMOVE)
if(NOT HAVE_MEMMOVE)
    message(WARNING "The memmove was NOT found")
endif()

check_cxx_symbol_exists(memset "string.h" HAVE_MEMSET)
if(NOT HAVE_MEMSET)
    message(WARNING "The memset was NOT found")
endif()

check_cxx_symbol_exists(ntohl "arpa/inet.h" HAVE_NTOHL)  # the definition and the declaration of the ntohl is at netinet/in.h, but arpa/inet.h includes it, so cmake can find it
if(NOT HAVE_NTOHL)
	message(WARNING "The ntohl can't be found")
endif()

check_cxx_symbol_exists(ntohs "arpa/inet.h" HAVE_NTOHS)
if(NOT HAVE_NTOHS)
    message(WARNING "The ntohs was NOT found")
endif()

check_cxx_symbol_exists(htons "arpa/inet.h" HAVE_HTONS)
if(NOT HAVE_HTONS)
    message(WARNING "The htons was NOT found")
endif()

set(build_examples FALSE)
if(have_nettle)
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		set(build_examples TRUE)
	endif()
endif()

set(ENABLE_EXAMPLES ${build_examples})

# TODOm: add the AC_CONFIG_FILES() command from the autotools
set(PACKAGE_VERSION ${PROJECT_VERSION})
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lib/includes/wslay/wslayver.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/lib/includes/wslay/wslayver.h" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lib/libwslay.pc.in" "${CMAKE_CURRENT_SOURCE_DIR}/lib/libwslay.pc" @ONLY)

add_subdirectory(lib)
include(tests/CPunit.cmake)

message(STATUS "Summary of build options:")
string(APPEND printKeys "\
	version;\
	Host;\
	Install Prefix;\
	C compiler;\
	CFlags;\
	Library types;\
	CUnit;\
	Nettle;\
	Build examples"
)

string(APPEND printValues "\
	${PROJECT_VERSION};\
	${CMAKE_SYSTEM_NAME};\
	${CMAKE_INSTALL_PREFIX};\
	${CC};\
	${CFlags};\
	${BUILD_SHARED_LIBS};\
	${have_cunit};\
	${have_nettle};\
	${build_examples}"
)
	# TODOm CC & CFLAGS, are whatever they have been setted from the initial CMakeLists.txt file

printBuildOptions(printKeys printValues)
