project(wslay LANGUAGES CXX C VERSION 1.1.1)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../cmake/")

# ####### Set compiler flags ########
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

include(GenerateConfigH)

config_h_add_compile_definitions(PACKAGE="${PROJECT_NAME}")
config_h_add_compile_definitions(PACKAGE_NAME="${PROJECT_NAME}")
config_h_add_compile_definitions(PACKAGE_STRING="${PROJECT_NAME} ${PROJECT_VERSION}")
config_h_add_compile_definitions(PACKAGE_TARNAME="${PROJECT_NAME}")
config_h_add_compile_definitions(PACKAGE_URL="${PROJECT_HOMEPAGE_URL}")
config_h_add_compile_definitions(PACKAGE_BUGREPORT="t-tujikawa@users.sourceforge.net")
config_h_add_compile_definitions(PACKAGE_VERSION="${PROJECT_VERSION}")
config_h_add_compile_definitions(VERSION="${PROJECT_VERSION}")

set(BUILD_ID "${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_HOST_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}")
set(HOST_ID "${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_HOST_SYSTEM_NAME}")
set(TARGET_ID "${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}")


config_h_add_compile_definitions(HOST="${HOST_ID}")
config_h_add_compile_definitions(BUILD="${BUILD_ID}")
config_h_add_compile_definitions(TARGET="${TARGET_ID}")

set(PACKAGE_VERSION ${PROJECT_VERSION})

option(ENABLE_WERROR "Builld with werror" off)

# #### Check for Libraries #####
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckFunctionExists)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
# using Clang
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
# using Intel C++
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  # config_h_add_compile_definitions(__MSVC__=1) # USE _MSC_VER
  add_compile_definitions(NOMINMAX)
  config_h_add_compile_macro("__restrict__ __restrict") 
  config_h_add_compile_macro("__attribute__(unused) /*Empty*/")
endif()

if(WIN32 AND NOT MINGW)
  set(NO_UNIX)
  config_h_add_compile_definitions(NO_UNIX=1)
endif()


if(ENABLE_WERROR)
    add_compile_options(-Wall -Werror -Wformat-security)
endif()

find_package(Sphinx)
if(Sphinx_FOUND)
    set(HAVE_SPHINX_BUILD true)
endif()

if(NOT NETTLE_FOUND) 
    # may have already been found and imported by parent project
    find_package(Nettle 2.4)
endif()

if(NETTLE_FOUND)
    set(HAVE_NETTLE true)
endif()

find_package(CUnit)
if(CUnit_FOUND)
    set(HAVE_CUNIT true)
endif()

if(WIN32)
    # Required for ntoh*/hton* functions.
    add_link_options(-lws2_32)
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} -lws2_32)
endif()

set(wslay_found_headers ${a2_found_headers}) #inherit from parent project if possible

set(_check_headers
    arpa/inet.h
    netinet/in.h
    stddef.h
    stdint.h
    stdlib.h
    string.h
    unistd.h
    memory.h
)

foreach(header ${_check_headers})
    string(TOUPPER ${header} header_var)
    string(REPLACE "." "_" header_var ${header_var})
    string(REPLACE "/" "_" header_var ${header_var})
    set(header_var "HAVE_${header_var}")
    set(${header_var})
    check_include_file(${header} ${header_var})

    if(${header_var})
        list(APPEND wslay_found_headers ${header})
        config_h_add_compile_definitions(${header_var})
    endif()
endforeach()

unset(_check_headers)

if(WIN32)
    # Need winsock2.h for ntoh*/hton* functions.
    check_include_file(winsock2.h HAVE_WINSOCK2_H)

    if(HAVE_WINSOCK2_H)
        config_h_add_compile_definitions(HAVE_WINSOCK2_H)
    endif()
endif()

set(save_CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES})
set(CMAKE_EXTRA_INCLUDE_FILES ${wslay_found_headers})
check_type_size(ptrdiff_t PTRDIFF_T LANGUAGE CXX)
if(PTRDIFF_T)
    set(HAVE_PTRDIFF_T true)
    config_h_add_compile_definitions(HAVE_PTRDIFF_T)
endif()
set(CMAKE_EXTRA_INCLUDE_FILES save_CMAKE_EXTRA_INCLUDE_FILES)

set(_check_funcs
    memmove
    memset
    ntohl
    ntohs
    htons
)

foreach(func ${_check_funcs})
  string(TOUPPER ${func} func_var)
  set(func_var "HAVE_${func_var}")
  set(${func_var})
  check_cxx_symbol_exists(${func} ${wslay_found_headers} ${func_var})

  if(${func_var})
    config_h_add_compile_definitions(${func_var})
  endif()
endforeach()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib/includes/wslay/wslayver.h.in ${CMAKE_CURRENT_SOURCE_DIR}/lib/includes/wslay/wslayver.h)

unset(_check_funcs)

if(HAVE_NETTLE AND UNIX)
    set(BUILD_EXAMPLES true)
    set(ENABLE_EXAMPLES true)
endif()


config_h_generate_header(config)
# pickup the generated config.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compile_definitions(HAVE_CONFIG_H)

add_subdirectory(lib)
add_subdirectory(tests)


message(STATUS "summary of build options:

    version:        ${PROJECT_VERSION} shared $LT_CURRENT:$LT_REVISION:$LT_AGE
    Host type:      ${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_HOST_SYSTEM}
    Install prefix: ${CMAKE_INSTALL_PREFIX}
    C compiler:     ${CMAKE_CXX_COMPILER}
    CFlags:         ${CMAKE_C_FLAGS}
    Library types:  Shared=${enable_shared}, Static=${enable_static}
    CUnit:          ${HAVE_CUNIT}
    Nettle:         ${HAVE_NETTLE}
    Build examples: ${BUILD_EXAMPLES}
")



########### install files ###############




#original Makefile.am contents follow:

## Wslay - The WebSocket Library
#
## Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa
#
## Permission is hereby granted, free of charge, to any person obtaining
## a copy of this software and associated documentation files (the
## "Software"), to deal in the Software without restriction, including
## without limitation the rights to use, copy, modify, merge, publish,
## distribute, sublicense, and/or sell copies of the Software, and to
## permit persons to whom the Software is furnished to do so, subject to
## the following conditions:
#
## The above copyright notice and this permission notice shall be
## included in all copies or substantial portions of the Software.
#
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
## EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
## MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
## NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
## LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
## OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
## WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#SUBDIRS = lib tests
#
#ACLOCAL_AMFLAGS = -I m4
