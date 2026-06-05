cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(aria2 VERSION 1.37.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # AC_USE_SYSTEM_EXTENSIONS

set(EXTRACFLAGS)
set(EXTRACXXFLAGS)
set(EXTRACPPFLAGS)
set(EXTRALDFLAGS)
set(EXTRALIBS)

if(MSVC) # check if the host is about to run in windows with mingw compiler
    EXTRALIBS

endif()

option(libuv "" TRUE)
option(appletls "" FALSE)
option(wintls "" FALSE)
option(gnutls "" FALSE)
option(libnettle "" FALSE)
option(libgmp "" FALSE)
option(libcrypt "" FALSE)
option(openssl "" FALSE)
option(sqlite3 "" FALSE)
option(libxml2 "" FALSE)
option(libexpat "" FALSE)
option(libcares "" FALSE)
option(libz "" FALSE)
option(tcmalloc "" TRUE)
option(jemalloc "" TRUE)
option(libssh2 "" FALSE)

option(ssl "" FALSE)
option(bittorrent "" FALSE)
option(metalink "" FALSE)
option(websocket "" FALSE)
option(epoll "" FALSE)
option(libaria2 "" FALSE)
option(werror "" FALSE)

option(ca-bundle "--with-ca-bundle=FILE, Use FILE as default CA bundle" OFF)
option(disk-cache "--with-disk-cache=SIZE, Use SIZE as the default disk-cache size")
option(bashcompletiondir "--with-bashcompletiondir=DIR, Directory to install bash_completion file" OFF)

option(ARIA2_STATIC "Set 'yes' to build a statically linked aria2" OFF)
option(gnutls-system-crypto-policy "--enable-gnutls-system-crypto-policy, Enable gnutls system wide crypto policy")

option(enable_gnutls_system_crypto_policy "Define to 1 if using gnutls system wide crypto policy" FALSE)
if(enable_gnutls_system_crypto_policy)
    add_compile_definitions(USE_GNUTLS_SYSTEM_CRYPTO_POLICY=1)
endif()


# should I include the module for the pkg-config here (?)
# include(pkg-config) # the 'pkg-config' is the name of the module, according to cmake doc
include(FindPkgConfig)
if(PKG_CONFIG_FOUND)
    if(${PKG_CONFIG_VERSION_STRING} VERSION_LESS 0.20)
        message(FATAL_ERROR "The current version of the pkg-config (${PKG_CONFIG_VERSION_STRING}) is less than 0.20. Try upgrade it first")
    endif()
    message(STATUS "pkg-config found with version ${PKG_CONFIG_VERSION_STRING}")
else()
    message(FATAL_ERROR "Cannot find pkg-config tool. Try installing it first")
endif()

# write a small test, to check if the compiler supports nullptr (akraio)

# write another small test, to check if the compiler supports `override` keyword
# if yes, define a new keyword CXX11_OVERRIDE, that will have the value of the 'override'
# if no, do nothing

if(ARIA2_STATIC)
    # check whether the platform is i686 or mingw
    # if() ...
    # endif()

    # set the pkg-config to produce static link libraries
    # list(APPEND ${PKG_CONFIG_ARGN} "--static")
endif()

pkg_check_modules(CPPUNIT REQUIRED "cppunit >= 1.10.2")
if(NOT CPPUNIT_FOUND)
    message(WARNING "The cppunit package not found. Try install it first")
endif()

# Check for libraries
include(CheckLibraryExists)

# Check for libz first needs more than tha
if(libz)
    find_package(zlib 1.2.3 REQUIRED)
    if(ZLIB_FOUND)
        message(STATUS "The libz found, with version ${ZLIB_VERSION}")
    else()
        message(WARNING "libz library is not installed. Try install it first")
    endif()
endif()

if(libuv)
    find_package(libuv 1.13 REQUIRED)
    if(LIBUV_FOUND)
        message(STATUS "The libuv found, with version ${LIBUV_VERSION}")
    else()
        message(WARNING "libuv library is not installed. Try install it first")
    endif()
endif()


if(libxml2)
    find_package(LibXml2 2.6.24 REQUIRED)
    if(LibXml2_FOUND)
        message(STATUS "The libxml2 found, with version ${LibXml2_VERSION}")
    else()
        message(WARNING "libxml2 library is not installd. Try install it first")
    endif()
endif()


if(libexpat)
    find_package(EXPAT REQUIRED)
    if(EXPAT_FOUND)
        message(STATUS "The EXPAT lib found, with version ${EXPAT_VERSION}")
        message(STATUS "Define to 1 if you have libexpat")
        add_compile_definitions(HAVE_LIBEXPAT=1)
        endif()
    else()
        message(WARNING "EXPAT library is not installd. Try install it first")
    endif()
endif()


if(sqlite3)
    find_package(SQLite3 REQUIRED)
    if(SQLite3_FOUND)
        message(STATUS "The sqlite3 found, with version ${SQLite3_VERSION}")
        message(STATUS "Define to 1 if you have sqlite3")
        add_compile_definitions(HAVE_SQLITE3=1)
        check_library_exists(${SQLite3_LIBRARIES} sqlite3_open_v2 "" sqlite3FuncFound)
        if(NOT sqlite3FuncFound)
            message(WARNING "Cannot find the requested function for sqlite3")
        endif()
    else()
        message(WARNING "Cannot find, the sqlite3 library")
    endif()
endif()
