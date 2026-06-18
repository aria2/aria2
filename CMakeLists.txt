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
option(libgcrypt "" FALSE)
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

include(CheckIncludeFiles)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(osx YES)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Mingw")
    check_include_files("windows.h" HAVE_WINDOWS_H)
    check_include_files("winsock.2.h" HAVE_WINSOCK2_H)
    check_include_files("ws2tcpip.h" HAVE_WS2TCPIP_H)
    check_include_files("mmsystem.h" HAVE_MMSYSTEM_H)
    check_include_files("io.h" HAVE_IO_H)
    check_include_files("iphlpapi.h" HAVE_IPHLPAPI_H)
    check_include_files("winioctl.h" HAVE_WINIOCTL_H)
    check_include_files("share.h" HAVE_SHARE_H)
endif()


if(NOT enable_ssl)
    set(appletls NO)
    set(wintls NO)
    set(libnettle NO)
    set(libgcrypt NO)
    set(gnutls NO)
    set(openssl NO)
endif()


set(have_appletls FALSE)
set(have_ssl FALSE)
set(have_nativetls FALSE)

if(appletls)
    message("Whether to enable Mac OS X native SSL/TLS")
    if(osx)
        message("Define to 1 if you have Apple TLS")
        add_compile_definitions(HAVE_APPLETLS=1)
        set(APPLETLS_LDFLAGS "-framework CoreFoundation -framework Security")
        # here somehow I have to pass this variable to the config.h.in file with @...@ symbols #
        set(have_appletls TRUE)
        set(have_ssl TRUE)
        set(have_nativetls TRUE)
    else()
        if(appletls)
            message("ARIA2_DEP_NOT_MET")
        endif()
    endif()
endif()

include(CheckLibraryExists)

set(have_wintls FALSE)
if(wintls)  ## the build is about windows
    find_package(crypt32 REQUIRED)
    if(crypt32_FOUND)
        check_library_exists(crypt32 main "" have_wintls_libs)
    endif()

    find_package(secur32 REQUIRED)
    if(secur32_FOUND)
        check_library_exists(secur32 main "" have_wintls_libs)
    endif()

    find_package(advapi32 REQUIRED)
    if(advapi32_FOUND)
        check_library_exists(advapi32 main "" have_wintls_libs)
    endif()

    include(CheckCSourceCompiles)
    check_c_source_compiles("
        #include <windows.h>
        #include <wincrypt.h>

        #ifndef SECURITY_WIN32
        #define SECURITY_WIN32 1
        #endif

        #include <security.h>


        int main(void) {
            return 0;
        }
    " HAVE_WINTLS_HEADERS)

    if(HAVE_WINTLS_HEADERS)
        set(have_wintls_headers YES)
    else()
        set(have_wintls_headers NO)
    endif()

    if(have_wintls_libs AND have_wintls_headers)
        add_compile_definitions(SECURITY_WIN32=1)
        message("Use security.h in WIN32 mode")
        set(WINTLS_LIBS "-lcrypt32 -lsecur32 -ladvapi32")
        # here somehow I have to pass this variable to the config.h.in file with @...@ symbols #
        set(have_wintls TRUE)
        set(have_ssl TRUE)
        set(have_nativetls TRUE)
    else()
        set(have_wintls FALSE)
    endif()

    if(NOT wintls)
        message("ARIA2_DEP_NOT_MET[wintls]")
    endif()
endif()

set(have_libgnutls FALSE)
if(gnutls AND NOT have_ssl)
    pkg_check_modules(LIBGNUTLS REQUIRED "gnutls >= 2.2.0")
    if(LIBGNUTLS_FOUND)
        set(have_libgnutls TRUE)
    endif()
    if(have_libgnutls)
        set(have_ssl TRUE)
        add_compile_definitions(HAVE_LIBGNUTLS=1)
        message("Define to 1 if you have libgnutls")
        check_library_exists(${LIBGNUTLS_LIBRARIES} gnutls_certificate_set_x509_system_trust "" gnuTLSFuncFound)
        if(NOT gnuTLSFuncFound)
            message(WARNING "The gnutls_certificate_set_x509_system_trust() cannot be found")
        endif()
    else()
        if(gnutls)
            message("ARIA2_DEP_NOT_MET([gnutls])")
        endif()
    endif()
endif()

set(have_openssl FALSE)
if(openssl AND NOT have_ssl)
    find_package(OpenSSL 0.9.8 REQUIRED)
    if(OpenSSL_FOUND)
        message(STATUS "The openssl library found, with version ${OpenSSL_VERSION}")
        set(have_openssl TRUE)
    else()
        message(WARNING "The openssl library was not found. Try install it first")
    endif()
    if(have_openssl)
        set(have_ssl TRUE)
        add_compile_definitions(HAVE_OPENSSL=1)
        message(STATUS "Define to 1 if you have openssl")

        # something here with the libs

        check_library_exists(${OPENSSL_LIBRARIES} EVP_DigestInit_ex "" have_digestinit_ex)
        if(NOT have_digestint_ex)
            add_compile_definitions(HAVE_OLD_OPENSSL=1)
            message(STATUS "Define to 1 if you have old openssl")
        endif()

        check_library_exists(${OPENSSL_LIBRARIES} EVP_sha224 "" have_evp_sha224)
        check_library_exists(${OPENSSL_LIBRARIES} EVP_sha256 "" have_evp_sha256)
        check_library_exists(${OPENSSL_LIBRARIES} EVP_sha384 "" have_evp_sha384)
        check_library_exists(${OPENSSL_LIBRARIES} EVP_sha512 "" have_evp_sha512)

        # again something here with the libs
    else()
        message(WARNING "Cannot find openssl in the system. Try install it first")
    endif()
endif()

set(have_libnettle FALSE)
set(have_libgmp FALSE)
set(have_libcrypt FALSE)

if(NOT have_openssl)
    if(libnettle AND NOT nativetls)
        pkg_check_modules(LIBNETTLE REQUIRED "libnettle8t64 > 1.1.1")  # den tha paiksei auto. Thelei allo tropo
        if(LIBNETTLE)
            add_compile_definitions(HAVE_LIBNETTLE=1)    
            message(STATUS "Define to 1 if you have libnettle")
        else()
            message(WARNING "ARIA2_DEP_NOT_MET")
        endif()
    endif()

    if(libgmp AND (libnettle OR have_nativetls) AND bittorrent)  # native_tls has to be placed into some other places in the code
        find_package(gmp REQUIRED)
        if(gmp_FOUND)
            check_library_exists(gmp __gmpz_init "" have_libgmp)
        endif()

        if(have_libgmp)
            # define here the LIBGMP_CFLAGS and etc
            #
            #
            add_compile_definitions(HAVE_LIBGMP=1)
            message(STATUS "Define to 1 if you have libgmp")
        else()
            message(WARNING "libgmp not found. Try install it first")
            message(WARNING "ARIA2_DEP_NOT_MET")
        endif()
    endif()

    if(libgcrypt AND NOT have_nativetls AND NOT have_libnettle)
    #
    #
    endif()
endif()

set(have_libssh2 FALSE)
if(libssh2)
    pkg_check_modules(LIBSSH2 REQUIRED "libssh2")
    if(LIBSSH2_FOUND)
        set(have_libssh2 TRUE)
        add_compile_definitions(HAVE_LIBSSH2=1)
        message(STATUS "Define to 1 if you have libssh2.")
    else()
        message(WARNING "The libssh2 library is missing")
        
        if(LIBSSH2_REQUESTED)
            message(WARNING "ARIA2_DEP_NOT_MET([libssh2])")
        endif()
    endif()
endif()


set(have_libcares FALSE)
