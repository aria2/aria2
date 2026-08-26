cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(aria2 VERSION 1.37.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # AC_USE_SYSTEM_EXTENSIONS

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Helpers)
include(FindPkgConfig)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckCSourceCompiles)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)
include(CheckStructHasMember)


set(EXTRACFLAGS)
set(EXTRACXXFLAGS)
set(EXTRACPPFLAGS)
set(EXTRALDFLAGS)
set(EXTRALIBS)

if(MSVC) # check if the host is about to run in windows with mingw compiler
    set(win_build TRUE)
    set(EXTRALIBS "-lws2_32 -lwsock32 -lgdi32 -lwinmm -liphlpapi -lpsapi ${EXTRALIBS}")

    # Define _POSIX_C_SOURCE to 1. This makes {asc,local}time_r
    # available from <time.h> even without (un)helpful interference
    # from <pthread.h>, and also defines __USE_MINGW_ANSI_STDIO.
    string(APPEND EXTRACPPFLAGS " -D_POSIX_C_SOURCE=1")

    # Build with ASLR (dynamicbase) and NX compatibility (nxcompat)
    # Enable pie once upstream/binutils gets fixed to produce correct
    # binaries with it.
    string(APPEND EXTRALDFLAGS " -Wl,--dynamicbase -Wl,--nxcompat")
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
option(tcmalloc "" FALSE)
option(jemalloc "" FALSE)
option(libssh2 "" FALSE)

option(ssl "" FALSE)
option(bittorrent "" FALSE)
option(metalink "" FALSE)
option(websocket "" FALSE)
option(epoll "" FALSE)
option(libaria2 "" FALSE)
option(werror "" FALSE)

set(ca_bundle "" CACHE FILEPATH "Set the path for the CA certificate")  # -Dca_bundle=path/to/cacert.crt file
set(disk_cache "" CACHE STRING "Use SIZE as the default disk-cache size")  # -Ddisk_cache=32
set(bashcompletiondir "" CACHE PATH "Directory to install bash_completion file")  #-Dbashcompletiondir=/home/user/

option(ARIA2_STATIC "Set 'yes' to build a statically linked aria2" NO)

# not necessary, since we are using the "enable_gnutls_system_crypto_policy" option
#option(gnutls-system-crypto-policy "--enable-gnutls-system-crypto-policy, Enable gnutls system wide crypto policy")

option(enable_gnutls_system_crypto_policy "Define to 1 if using gnutls system wide crypto policy" FALSE)
if(enable_gnutls_system_crypto_policy)
    add_compile_definitions(USE_GNUTLS_SYSTEM_CRYPTO_POLICY=1)
endif()

if(NOT CMAKE_AR)
	message(FATAL_ERROR "ar is not found in the system.")
endif()

find_program(SPHINXBUILD sphinx-build)  # sudo apt-get install sphinx
if(SPHINXBUILD)
    message(STATUS "Found Sphinx: ${SPHINXBUILD}")
    set(HAVE_SPHINXBUILD TRUE CACHE BOOL "sphinx-build was found" FORCE)
else()
    message(WARNING "sphinx-build program not found. Install it with: sudo apt-get install sphinx")
endif()

find_program(RST2HTML NAMES rst2html rst2html.py)  # sudo pip install rst2html
if(RST2HTML)
    message(STATUS "Found Sphinx: ${RST2HTML}")
    set(HAVE_RST2HTML TRUE CACHE BOOL "rst2html was found" FORCE)
else()
    message(STATUS "Sphinx not found")
    message(WARNING "rst2html program was not found. Install it with: sudo pip install rst2html")
endif()

if(PKG_CONFIG_FOUND)
    if(${PKG_CONFIG_VERSION_STRING} VERSION_LESS 0.20)
        message(FATAL_ERROR "The current version of the pkg-config (${PKG_CONFIG_VERSION_STRING}) is less than 0.20. Try upgrade it first")
    endif()
    message("pkg-config found with version ${PKG_CONFIG_VERSION_STRING}")
else()
    message(FATAL_ERROR "Cannot find pkg-config tool. Try installing it first")
endif()

message("Checking whether the C++ compiler supports nullptr")
set(nullptrTest
    [==[
        #include <iostream>

        int main() {
            int *a = nullptr;
            return 0;
        }
    ]==]
)
check_cxx_source_compiles("${nullptrTest}" SUPPORTS_NULLPTR)
if(SUPPORTS_NULLPTR)
    message("C++ compiler supports nullptr keyword")
else()
    message(FATAL_ERROR "C++ compiler does not understand nullptr, perhaps C++ compiler is too old. Try again with new one (gcc >= 4.8.3 or clang >= 3.4)")
endif()

message("Checking whether C++ compiler supports 'override' keyword")
set(overrideTest
    [==[
        #include <iostream>

        struct Base {
            virtual void f() = 0;
        };

        struct Derived : Base {
            virtual void f() override {}
        };

        int main() {
            Derived x;
            int a = 77;  // to make sure that it goes beyond the Derived x; line
            return 0;
        }
    ]==]
)

check_cxx_source_compiles("${overrideTest}" SUPPORTS_OVERRIDE)
if(SUPPORTS_OVERRIDE)
    message("C++ compiler supports override keyword")
    add_compile_definitions(CXX11_OVERRIDE=override)
    message("Define `override` keyword if the compiler supports it")
endif()


if(ARIA2_STATIC)
    # check whether the platform is i686 or mingw
	if(MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 4) # TODOm: equal to i686*mingw => searches for a 32-bit MinGW target - replaces i686*mingw from configure.ac file
        string(APPEND EXTRACPPFLAGS " -D_USE_32BIT_TIME_T")
    endif()

    # set the pkg-config to produce static link libraries
    string(APPEND ${PKG_CONFIG_ARGN} " --static")
else()
    set(ARIA2_STATIC NO)
endif()

pkg_check_modules(CPPUNIT "cppunit >= 1.10.2")
if(NOT CPPUNIT_FOUND)
    message(WARNING "The cppunit package not found. Try install it first")
endif()

set(have_zlib FALSE)
if(libz)
    pkg_check_modules(ZLIB "zlib >= 1.2.3")
    if(NOT ZLIB_FOUND)
        message(WARNING "Couldn't find zlib.pc file. Try checking if the zlib.h exists")
        set(save_LIBS "${CMAKE_REQUIRED_LIBRARIES}")
        set(CMAKE_REQUIRED_LIBRARIES "z ${CMAKE_REQUIRED_LIBRARIES}")
        check_symbol_exists(zlibVersion "zlib.h" have_zlib)
        if(have_zlib)
            message("The zlib.h header found")
            set(ZLIB_CFLAGS "")
            set(ZLIB_LIBS "-lz")
        else()
            message(WARNING "ARIA2_DEP_NOT_MET(libz)")
        endif()
        set(CMAKE_REQUIRED_LIBRARIES "${save_LIBS}")
    endif()

    if(ZLIB_FOUND)
        set(have_zlib TRUE)
        add_compile_definitions(HAVE_ZLIB=1)
        set(save_LIBS "${CMAKE_REQUIRED_LIBRARIES}")
        set(CMAKE_REQUIRED_LIBRARIES "${ZLIB_LIBRARIES} ${CMAKE_REQUIRED_LIBRARIES}")
        check_symbol_exists(gzbuffer "zlib.h" HAVE_GZBUFFER)
        if(HAVE_GZBUFFER)
            message("The gzbuffer found")
        endif()
        set(CMAKE_REQUIRED_LIBRARIES "${save_LIBS}")
	endif()
endif()

set(have_libuv FALSE)
if(libuv)
    pkg_check_modules(LIBUV "libuv >= 1.13")
    if(LIBUV_FOUND)
        set(have_libuv TRUE)
        add_compile_definitions(HAVE_LIBUV=1)
        set(HAVE_LIBUV TRUE CACHE BOOL "LibUV was found" FORCE)
    else()
        message(WARNING "ARIA2_DEP_NOT_MET([libuv])")
    endif()
endif()


set(have_libxml2 FALSE)
if(libxml2)
    # find_package(LibXml2 2.6.24)
    pkg_check_modules(LibXml2 "libxml-2.0 >= 2.6.24")  # to install it: sudo apt-get install libxml2-dev
    if(LibXml2_FOUND)
        set(have_libxml2 TRUE)
        add_compile_definitions(HAVE_LIBXML2=1)
    else()
        message(WARNING "ARIA2_DEP_NOT_MET([libxml2]). Install it with: sudo apt-get install libxml2-dev")
    endif()
endif()

set(have_libexpat FALSE)
if(libexpat AND NOT ${have_libxml2})
    # find_package(EXPAT REQUIRED)
    pkg_check_modules(EXPAT "expat") # to install it: sudo apt-get install libexpat1-dev
    if(EXPAT_FOUND)
        set(have_libexpat TRUE)
        add_compile_definitions(HAVE_LIBEXPAT=1)
    else()
        message(WARNING "ARIA2_DEP_NOT_MET([libexpat]). Install it with: sudo apt-get install libexpat1-dev")
    endif()
endif()

set(have_sqlite3 FALSE)
if(sqlite3)
    # find_package(SQLite3 REQUIRED)
    pkg_check_modules(SQLite3 "sqlite3") # to install it: sudo apt-get install libsqlite3-dev
    if(SQLite3_FOUND)
        set(have_sqlite3 TRUE)
        add_compile_definitions(HAVE_SQLITE3=1)
        check_library_exists(${SQLite3_LIBRARIES} sqlite3_open_v2 "" sqlite3FuncFound)
        if(NOT sqlite3FuncFound)
            message(WARNING "Cannot find the function: sqlite3_open_v2() from sqlite3")
        endif()
    else()
        message(WARNING "ARIA2_DEP_NOT_MET([sqlite3]). Install it with: sudo apt-get install libsqlite3-dev")
    endif()
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(have_osx YES)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Mingw")
    check_include_files("ws2tcpip.h" HAVE_WS2TCPIP_H)
    check_include_files("winsock.2.h" HAVE_WINSOCK2_H)
    check_include_files("windows.h" HAVE_WINDOWS_H)
    # check_include_files("mmsystem.h" HAVE_MMSYSTEM_H)
    # check_include_files("io.h" HAVE_IO_H)
    # check_include_files("iphlpapi.h" HAVE_IPHLPAPI_H)
    # check_include_files("winioctl.h" HAVE_WINIOCTL_H)
    # check_include_files("share.h" HAVE_SHARE_H)
endif()


if(NOT enable_ssl)
    set(appletls NO)
    set(wintls NO)
    set(libnettle NO)
    set(libgcrypt NO)
    set(gnutls NO)
    set(openssl NO)
endif()


set(have_nativetls FALSE)
set(have_appletls FALSE)
if(appletls)
    message("Whether to enable Mac OS X native SSL/TLS")
    if(have_osx)
        message("Define to 1 if you have Apple TLS")
        add_compile_definitions(HAVE_APPLETLS=1)
        string(APPEND APPLETLS_LDFLAGS "-framework CoreFoundation -framework Security")

        set(have_appletls TRUE)
        set(have_ssl TRUE)
        set(have_nativetls TRUE)
    else()
        message("ARIA2_DEP_NOT_MET([appletls])")
    endif()
endif()

set(have_wintls FALSE)
if(wintls)
    find_package(crypt32 REQUIRED)
    if(crypt32_FOUND)
        check_library_exists(crypt32 main "" have_crypt32)
    endif()

    find_package(secur32 REQUIRED)
    if(secur32_FOUND)
        check_library_exists(secur32 main "" have_secur32)
    endif()

    find_package(advapi32 REQUIRED)
    if(advapi32_FOUND)
        check_library_exists(advapi32 main "" have_advapi32)
    endif()

    if(${have_crypt32} AND ${have_secur32} AND ${have_advapi32})
        set(have_wintls_libs TRUE)
    endif()

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

    if(HAVE_WINTLS_HEADERS AND have_wintls_libs)
        set(have_wintls_headers TRUE)
    else()
        set(have_wintls_headers FALSE)
    endif()

    if(have_wintls_libs AND have_wintls_headers)
        add_compile_definitions(SECURITY_WIN32=1)
        # message("Use security.h in WIN32 mode")
        set(WINTLS_LIBS "-lcrypt32 -lsecur32 -ladvapi32")

        set(have_wintls TRUE)
        set(have_ssl TRUE)
        set(have_nativetls TRUE)
    else()
        set(have_wintls FALSE)
    endif()

    if(NOT have_wintls AND wintls)
        message("ARIA2_DEP_NOT_MET[wintls]")
    endif()
endif()

set(have_libgnutls FALSE)
if(gnutls AND NOT have_ssl)
    # find_package(GnuTLS 2.2.0)
    pkg_check_modules(GnuTLS REQUIRED "gnutls >= 2.2.0")  # sudo apt-get install gnutls-dev
    if(GnuTLS_FOUND)
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
        message("ARIA2_DEP_NOT_MET([gnutls]). Install it with: sudo apt-get install gnutls-dev")
    endif()
endif()

set(have_openssl FALSE)
if(openssl AND NOT have_ssl)
    # find_package(OpenSSL 0.9.8 REQUIRED)
    pkg_check_modules(OpenSSL REQUIRED "openssl >= 0.9.8")  # sudo apt-get install libssl-dev
    if(OpenSSL_FOUND)
        set(have_openssl TRUE)
    endif()

    if(have_openssl)
        set(have_ssl TRUE)
        add_compile_definitions(HAVE_OPENSSL=1)
        set(save_LIBS "${CMAKE_REQUIRED_LIBRARIES}")
        string(APPEND CMAKE_REQUIRED_LIBRARIES " ${OpenSSL_LIBRARIES}")
        
        check_symbol_exists(EVP_DigestInit_ex "openssl/evp.h" have_digestinit_ex)
        if(have_digestinit_ex)
            add_compile_definitions(HAVE_OLD_OPENSSL=1)
            message("Define to 1 if you have old openssl")
        endif()

        check_symbol_exists(EVP_sha224 "openssl/evp.h" have_evp_sha224)
        check_symbol_exists(EVP_sha256 "openssl/evp.h" have_evp_sha256)
        check_symbol_exists(EVP_sha384 "openssl/evp.h" have_evp_sha384)
        check_symbol_exists(EVP_sha512 "openssl/evp.h" have_evp_sha512)

        set(CMAKE_REQUIRED_LIBRARIES "${save_LIBS}")

    else()
        message(WARNING "ARIA2_DEP_NOT_MET([openssl]). Install it with: sudo apt-get install libssl-dev")
    endif()
endif()

set(have_libnettle FALSE)
set(have_libgmp FALSE)
set(have_libcrypt FALSE)
if(NOT have_openssl)
    if(libnettle AND NOT have_nativetls)
        pkg_check_modules(LIBNETTLE REQUIRED "nettle") # sudo apt-get install nettle-dev
        if(LIBNETTLE_FOUND)
            # message("The libnettle found with version ${LIBNETTLE_VERSION} at path ${LIBNETTLE_INCLUDE_DIRS}")
            set(have_libnettle TRUE)
            add_compile_definitions(HAVE_LIBNETTLE=1)
        else()
            message(WARNING "ARIA2_DEP_NOT_MET([nettle]). Install it with: sudo apt-get install nettle-dev")
        endif()
    endif()

    if(libgmp AND (have_libnettle OR have_nativetls) AND bittorrent)  # DONE: native_tls has to be placed into some other places in the code
        pkg_check_modules(GmpLib REQUIRED "gmp")  # apt-get install libgmp-dev OR libgmp3-dev
        set(save_LIBS "${CMAKE_REQUIRED_LIBRARIES}")
        string(APPEND CMAKE_REQUIRED_LIBRARIES " ${GmpLib_LIBRARIES}")
        check_symbol_exists(__gmpz_init "gmp.h" have_libgmp)
        if(have_libgmp)
            set(LIBGMP_CFLAGS "")
            set(LIBGMP_LIBS "-lgmp")

            add_compile_definitions(HAVE_LIBGMP=1)
            check_symbol_exists(__gmpz_powm_sec "gmp.h" have_mpz_powm_sec)
            if(have_mpz_powm_sec)            
                add_compile_definitions(HAVE_GMP_SEC=1)
            endif()

        else()
            message(WARNING "libgmp not found")
            message(WARNING "ARIA2_DEP_NOT_MET([libgmp]). Install it with: apt-get install libgmp-dev")
        endif()
        set(CMAKE_REQUIRED_LIBRARIES "${save_LIBS}")
    endif()

    if(libgcrypt AND NOT have_nativetls AND NOT have_libnettle)
    # TODO: lines 507 - 517 from configure.ac file. 
    # TODO: hard one.
    # if test "x$with_libgcrypt" = "xyes" &&
    #         test "x$have_nativetls" != "xyes" &&
    #         test "x$have_libnettle" != "xyes"; then
    #         m4_ifdef([AM_PATH_LIBGCRYPT], [AM_PATH_LIBGCRYPT([1.2.4], [have_libgcrypt=yes])], [
    #         AC_MSG_WARN([configure was generated without libgcrypt detection.  libgcrypt detection is disabled])
    #         LIBGCRYPT_CFLAGS=
    #         LIBGCRYPT_LIBS=
    #         AC_SUBST([LIBGCRYPT_CFLAGS])
    #         AC_SUBST([LIBGCRYPT_LIBS])
    #     ])
    #     if test "x$have_libgcrypt" = "xyes"; then
    #     AC_DEFINE([HAVE_LIBGCRYPT], [1], [Define to 1 if you have libgcrypt.])
    #     fi
    # fi
    endif()
endif()

set(have_libssh2 FALSE)
if(libssh2)
    pkg_check_modules(LIBSSH2 REQUIRED "libssh2")  # sudo apt-get install libssh2-1-dev
    if(LIBSSH2_FOUND)
        set(have_libssh2 TRUE)
        add_compile_definitions(HAVE_LIBSSH2=1)

        if(ARIA2_STATIC)
            string(APPEND LIBSSH2_CFLAGS " -DLIBSSH2_API")
        endif()
    else()
        message(WARNING "ARIA2_DEP_NOT_MET([libssh2]). Install it with: sudo apt-get install libssh2-1-dev")
    endif()
endif()


set(have_libcares FALSE)
if(libcares)
    pkg_check_modules(LIBCARES "libcares >= 1.16.0")  # sudo apt-get install libc-ares-dev
    if(LIBCARES_FOUND)
            set(have_libcares TRUE)
            add_compile_definitions(HAVE_LIBCARES=1)
    else()
            message(WARNING "ARIA2_DEP_NOT_MET([libcares). Install it with: sudo apt-get install libc-ares-dev")
    endif()
endif()

set(use_md "")
if(have_appletls)
    set(use_md "apple")
    message("What message digest implementation to use")
    add_compile_definitions(USE_APPLE_MD=1)
elseif(have_libnettle)
    add_compile_definitions(USE_LIBNETTLE_MD=1)
    message("What message digest implementation to use")
    set(use_md "libnettle")
elseif(have_libcrypt)
    add_compile_definitions(USE_LIBCRYPT_MD=1)
    message("What message digest implementation to use")
    set(use_md "libcrypt")
elseif(have_openssl)
    add_compile_definitions(USE_OPENSSL_MD=1)
    message("What message digest implementation to use")
    set(use_md "openssl")
else()
    add_compile_definitions(USE_INTERNAL_MD=1)
    message("What message digest implementation to use")
    set(use_md "internal")
endif()

if(have_ssl)
    add_compile_definitions(ENABLE_SSL=1)
    set(ENABLE_SSL TRUE CACHE BOOL "ssh was found" FORCE)
else()
    set(have_ssl no)
    set(ENABLE_SSL FALSE CACHE BOOL "ssh was not found" FORCE)
endif()

# this can be done more clearly with a list of the variables that I want to check if they
# are true or false.

if(have_osx)
    set(HAVE_OSX TRUE CACHE BOOL "osx was found" FORCE)
endif()
if(have_appletls)
    set(HAVE_APPLETLS TRUE CACHE BOOL "appletls was found" FORCE)
endif()
if(have_wintls)
    set(HAVE_WINTLS TRUE CACHE BOOL "wintls was found" FORCE)
endif()
if(use_md STREQUAL "apple")
    set(USE_APPLE_MD TRUE CACHE BOOL "md is equal to apple" FORCE)
endif()
if(use_md STREQUAL "windows")
    set(USE_WINDOWS_MD TRUE CACHE BOOL "md is equal to windows" FORCE)
endif()
if(have_libgnutls)
    set(HAVE_LIBGNUTLS TRUE CACHE BOOL "gnutls is enabled" FORCE)
endif()
if(have_libnettle)
    set(HAVE_LIBNETTLE TRUE CACHE BOOL "libnettle is enabled" FORCE)
endif()
if(use_md STREQUAL "libnettle")
    set(USE_LIBNETTLE_MD TRUE CACHE BOOL "md is equal to libnettle" FORCE)
endif()
if(have_libgmp)
    set(HAVE_LIBGMP TRUE CACHE BOOL "libgmp was found" FORCE)
endif()
if(libgcrypt)
    set(HAVE_LIBGCRYPT TRUE CACHE BOOL "libgcrypt was found" FORCE)
endif()
if(use_md STREQUAL "libgcrypt")
    set(USE_LIBGCRYPT_MD TRUE CACHE BOOL "md is equal to libgcrypt" FORCE)
endif()
if(have_openssl)
    set(HAVE_OPENSSL TRUE CACHE BOOL "openssl was found" FORCE)
endif()
if(use_md STREQUAL "openssl")
    set(USE_OPENSSL_MD TRUE CACHE BOOL "md is equal to openssl" FORCE)
endif()
if(use_md STREQUAL "internal")
    set(USE_INTERNAL_MD TRUE CACHE BOOL "md is equal to internal" FORCE)
endif()

if(have_libgmp OR have_libgcrypt OR have_openssl)
    set(USE_INTERNAL_BIGNUM FALSE CACHE BOOL "internal bignum set to FALSE" FORCE)
else()
    add_compile_definitions(USE_INTERNAL_BIGNUM=1)
    set(USE_INTERNAL_BIGNUM TRUE CACHE BOOL "internal bignum set to TRUE" FORCE)
endif()

if(have_libnettle OR have_libgcrypt OR have_openssl)
    set(USE_INTERNAL_ARC4 FALSE CACHE BOOL "internal arc4 set to FALSE" FORCE)
else()
    add_compile_definitions(USE_INTERNAL_ARC4=1)
    set(USE_INTERNAL_ARC4 TRUE CACHE BOOL "internal arc4 set to TRUE" FORCE) # TODO: uses the AM_CONDITIONAL
endif()

if(bittorrent)
    add_compile_definitions(ENABLE_BITTORRENT=1)
    set(ENABLE_BITTORRENT TRUE CACHE BOOL "bittorrent set to TRUE" FORCE)
else()
    if(bittorrent)
        message(WARNING "ARIA2_FET_NOT_SUPPORTED([bittorrent])")
    endif()
    set(bittorrent FALSE)
    set(ENABLE_BITTORRENT FALSE CACHE BOOL "bittorrent set to FALSE" FORCE)
endif()

if((have_libxml2 OR have_libexpat) AND metalink)
    add_compile_definitions(ENABLE_METALINK=1)
    set(ENABLE_METALINK TRUE CACHE BOOL "metalink is enabled" FORCE)
else()
    if(metalink)
        message(WARNING "ARIA2_FET_NOT_SUPPORTED([metalink])")
    endif()
    set(metalink FALSE)
    set(ENABLE_METALINK FALSE CACHE BOOL "metalink is disabled" FORCE)
endif()

if(have_libxml2 OR have_libexpat)
    set(HAVE_SOME_XMLLIB TRUE CACHE BOOL "xml2 or expat exists" FORCE)
    set(enable_xml_rpc TRUE)
else()
    set(enable_xml_rpc FALSE)
endif()

if(enable_xml_rpc)
    add_compile_definitions(ENABLE_XML_RPC=1)
    set(ENABLE_XML_RPC TRUE CACHE BOOL "xml_rpc is enabled" FORCE)

endif()

if(have_libxml2)
    set(HAVE_LIBXML2 TRUE CACHE BOOL "xlm2 is enabled" FORCE)
endif()

if(have_libexpat)
    set(HAVE_LIBEXPAT TRUE CACHE BOOL "expat is enabled" FORCE)
endif()

if(have_libcares)
    add_compile_definitions(ENABLE_ASYNC_DNS=1)
    set(ENABLE_ASYNC_DNS TRUE CACHE BOOL "c-ares was found" FORCE)
else()
    set(ENABLE_ASYNC_DNS FALSE CACHE BOOL "c-ares was not found" FORCE)
endif()

if(have_zlib)
    set(HAVE_ZLIB TRUE CACHE BOOL "z-lib was found" FORCE)
endif()

if(have_sqlite3)
    set(HAVE_SQLITE3 TRUE CACHE BOOL "sqlite3 was found" FORCE)
endif()

if(have_libssh2)
    set(HAVE_LIBSSH2 TRUE CACHE BOOL "ssh2 was found" FORCE)
endif()

if(CMAKE_HOST_SOLARIS)

    set(save_LIBS "${CMAKE_REQUIRED_LIBS}")
        
    # TODOm: should recheck the below case that is for solaris OS
    # case "$host" in
    # *solaris*)
    #     save_LIBS=$LIBS
    #     LIBS=
    #     AC_SEARCH_LIBS([getaddrinfo], [nsl socket], [], [$save_LIBS])
    #     EXTRALIBS="$LIBS $EXTRALIBS"
    #     LIBS=$save_LIBS
    #     ;;
    # esac
endif()

check_include_file(alloca.h HAVE_ALLOCA_HEADER)
if(HAVE_ALLOCA_HEADER)
	message("The alloca.h exists")
    add_compile_definitions(HAVE_ALLOCA_H=1)
endif()

# after the alloca.h header should be the alloca function
check_symbol_exists(alloca "alloca.h" HAVE_ALLOCA_FUNC)
if(HAVE_ALLOCA_FUNC)
	message("The alloca() was found")
endif()


list(APPEND HEADER_FILES argz.h; arpa/inet.h; fcntl.h; float.h; inttypes.h; langinfo.h; libintl.h; limits.h; locale.h; malloc.h;
                        netdb.h; netinet/in.h; netinet/tcp.h; poll.h; signal.h; stddef.h; stdint.h; stdio_ext.h;
                        stdlib.h; string.h; strings.h; sys/ioctl.h; sys/param.h; sys/resource.h; sys/signal.h; sys/socket.h; sys/time.h; sys/types.h; sys/uio.h; sys/utsname.h;
                        termios.h; unistd.h; utime.h; wchar.h; ifaddrs.h; pwd.h
)

check_include_files("${HEADER_FILES}" HAVE_HEADER_FILES)

check_include_file(port.h HAVE_PORT_HEADER)
if(HAVE_PORT_HEADER)
    add_compile_definitions(HAVE_PORT_H=1)
endif()

check_type_size(size_t SIZEOF_SIZE_T)
if(HAVE_SIZEOF_SIZE_T)
	message(STATUS "The size_t exists with size ${SIZEOF_SIZE_T}")
endif()

check_type_size(ssize_t SSIZEOF_SIZE_T)
if(HAVE_SSIZEOF_SIZE_T)
    message(STATUS "The ssize_t exists with size: ${SSIZEOF_SIZE_T}")
endif()

check_include_file("sys/time.h" HAVE_SYS_TIME)  # sys/time.h implied to: /usr/include/x86_64-linux-gnu/sys/time.h
if(NOT HAVE_SYS_TIME)
    message(WARNING "The sys/time.h was not found")
endif()

# in order to be able to find the C struct, I have to enable C language
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD_REQUIRED TRUE)
# now it will be able to find the c struct
check_struct_has_member(
    "struct timespec"
    tv_sec
    time.h
    have_timespec
)

if(have_timespec)
    add_compile_definitions(HAVE_A2_STRUCT_TIMESPEC=1)
endif()

find_package(Gettext 0.19)  # apt-get install gettext, if doesn't exist in the system
find_package(Intl)
if(NOT Gettext_FOUND OR NOT Intl_FOUND)
    message(WARNING "The gettext or libintl was NOT found: Install it with: sudo apt-get install gettext")
endif()

# verify that libintl.h header exists
check_include_file(libintl.h LIBINTL_EXISTS)
if(NOT LIBINTL_EXISTS)
    message(WARNING "Header libintl.h not found")
endif()

check_cxx_symbol_exists(error_at_line "error.h" HAVE_ERROR_AT_LINE)
if(NOT HAVE_ERROR_AT_LINE)
    message(WARNING "error_at_line function was NOT found")
endif()

check_cxx_symbol_exists(memcmp "cstring" HAVE_MEMCMP)
if(NOT HAVE_MEMCMP)
    message(WARNING "memcmp function was NOT found")
endif()

check_cxx_symbol_exists(mktime "ctime" HAVE_MKTIME)
if(NOT HAVE_MKTIME)
    message(WARNING "mktime was NOT found")
endif()

# AC_FUNC_SELECT_ARGTYPES  # OBSOLETE option. Not included here

check_cxx_symbol_exists(stat "sys/stat.h" HAVE_STAT)
if(NOT HAVE_STAT)
    message(WARNING "stat was NOT found")
endif()

check_cxx_symbol_exists(strftime "ctime" HAVE_STRFTIME)
if(NOT HAVE_STRFTIME)
    message(WARNING "strftime was NOT found")
endif()

check_cxx_symbol_exists(vprintf "cstdio" HAVE_VPRINTF )
if(NOT HAVE_VPRINTF)
    message(WARNING "vrpintf was NOT found")
endif()

check_cxx_symbol_exists(fork "unistd.h" HAVE_FORK)
if(NOT HAVE_FORK)
    message(WARNING "fork was NOT found")
endif()

check_cxx_symbol_exists(strtod "stdlib.h" HAVE_STRTOD)
if(NOT HAVE_STRTOD)
    message(WARNING "strtod was NOT found")
endif()

check_cxx_symbol_exists(__argz_count "argz.h" HAVE_ARGZ_COUNT)
if(NOT HAVE_ARGZ_COUNT)
    message(WARNING "The argz was NOT found")
endif()

check_cxx_symbol_exists(__argz_next "argz.h" HAVE_ARGZ_NEXT)
if(NOT HAVE_ARGZ_NEXT)
    message(WARNING "The argz_next was NOT found")
endif()

check_cxx_symbol_exists(__argz_stringify "argz.h" HAVE_STRINGIFY)
if(NOT HAVE_STRINGIFY)
    message(WARNING "The __argz_stringify was NOT found")
endif()

check_cxx_symbol_exists(atexit "stdlib.h" HAVE_ATEXIT)
if(NOT HAVE_ATEXIT)
    message(WARNING "The atexit was NOT found")
endif()

check_cxx_symbol_exists(ftruncate "unistd.h" HAVE_FTRUNCATE)
if(NOT HAVE_FTRUNCATE)
    message(WARNING "The ftruncate was NOT found")
endif()

check_cxx_symbol_exists(getcwd "unistd.h" HAVE_GETCWD)
if(NOT HAVE_GETCWD)
    message(WARNING "The getcwd was NOT found")
endif()

check_cxx_symbol_exists(getentropy "unistd.h" HAVE_ENTROPY)
if(NOT HAVE_ENTROPY)
    message(WARNING "The getentropy was NOT found")
endif()

check_cxx_symbol_exists(gethostbyaddr "netdb.h" HAVE_HOSTBYADDR)
if(NOT HAVE_HOSTBYADDR)
    message(WARNING "The gethostbyaddr was NOT found")
endif()

check_cxx_symbol_exists(gethostbyname "netdb.h" HAVE_HOSTBYNAME)
if(NOT HAVE_HOSTBYNAME)
    message(WARNING "The gethostbyname was NOT found")
endif()

check_cxx_symbol_exists(getifaddrs "ifaddrs.h" HAVE_IFADDRS)
if(NOT HAVE_IFADDRS)
    message(WARNING "The getifaddrs was NOT found")
endif()

check_cxx_symbol_exists(getpagesize "unistd.h" HAVE_PAGESIZE)
if(NOT HAVE_PAGESIZE)
    message(WARNING "The getpagesize function was NOT found")
endif()

check_symbol_exists(memchr "string.h" HAVE_MEMCHR)
if(NOT HAVE_MEMCHR)
    message(WARNING "The memchr was NOT found")
endif()

check_cxx_symbol_exists(memmove "string.h" HAVE_MEMMOVE)
if(NOT HAVE_MEMMOVE)
    message(WARNING "The memmove was NOT found")
endif()

check_cxx_symbol_exists(mempcpy "string.h" HAVE_MEMPCPY)
if(NOT HAVE_MEMPCPY)
    message(WARNING "The mempcpy was NOT found")
endif()

check_cxx_symbol_exists(memset "string.h" HAVE_MEMSET)
if(NOT HAVE_MEMSET)
    message(WARNING "The memset was NOT found")
endif()

check_cxx_symbol_exists(mkdir "sys/stat.h" HAVE_MKDIR)
if(NOT HAVE_MKDIR)
    message(WARNING "The mkdir was NOT found")
endif()

check_cxx_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
if(NOT HAVE_MMAP)
    message(WARNING "The mmap was NOT found")
endif()

check_cxx_symbol_exists(munmap "sys/mman.h" HAVE_MUNMAP)
if(NOT HAVE_MUNMAP)
    message(WARNING "The munmap was NOT found")
endif()

check_cxx_symbol_exists(nl_langinfo "langinfo.h" HAVE_LANGINFO)
if(NOT HAVE_LANGINFO)
    message(WARNING "The nl_langinfo was NOT found")
endif()

check_cxx_symbol_exists(posix_fadvise "fcntl.h" HAVE_POSIXFADVISE)
if(NOT HAVE_POSIXFADVISE)
    message(WARNING "The posix_fadvise was NOT found")
endif()

check_cxx_symbol_exists(posix_memalign "stdlib.h" HAVE_POSIXMEMALIGN)
if(NOT HAVE_POSIXMEMALIGN)
    message(WARNING "The posix_memalign was NOT found")
endif()

check_cxx_symbol_exists(pow "tgmath.h" HAVE_POW)
if(NOT HAVE_POW)
    message(WARNING "The pow was NOT found")
endif()

check_cxx_symbol_exists(putenv "stdlib.h" HAVE_PUTENV)
if(NOT HAVE_PUTENV)
    message(WARNING "The putenv was NOT found")
endif()

check_cxx_symbol_exists(rmdir "unistd.h" HAVE_RMDIR)
if(NOT HAVE_RMDIR)
    message(WARNING "The rmdir was NOT found")
endif()

check_cxx_symbol_exists(select "sys/select.h" HAVE_SELECT)
if(NOT HAVE_SELECT)
    message(WARNING "The select was NOT found")
endif()

check_cxx_symbol_exists(setlocale "locale.h" HAVE_LOCALE)
if(NOT HAVE_LOCALE)
    message(WARNING "The setlocale was NOT found")
endif()

check_cxx_symbol_exists(sigaction "signal.h" HAVE_SIGACTION)
if(NOT HAVE_SIGACTION)
    message(WARNING "The sigaction was NOT found")
endif()

check_cxx_symbol_exists(sleep "unistd.h" HAVE_SLEEP)
if(NOT HAVE_SLEEP)
    message(WARNING "The sleep was NOT found")
endif()

check_cxx_symbol_exists(socket "sys/socket.h" HAVE_SOCKET)
if(NOT HAVE_SOCKET)
    message(WARNING "The socket was NOT found")
endif()

check_cxx_symbol_exists(stpcpy "string.h" HAVE_STPCPY)
if(NOT HAVE_STPCPY)
    message(WARNING "The stpcpy was NOT found")
endif()

check_cxx_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)
if(NOT HAVE_STRCASECMP)
    message(WARNING "The strcasecmp was NOT found")
endif()

check_symbol_exists(strchr "string.h" HAVE_STRCHR)
if(NOT HAVE_STRCHR)
    message(WARNING "The strchr was NOT found")
endif()

check_cxx_symbol_exists(strcspn "string.h" HAVE_STRCSPN)
if(NOT HAVE_STRCSPN)
    message(WARNING "The strcspn was NOT found")
endif()

check_cxx_symbol_exists(strdup "string.h" HAVE_STRDUP)
if(NOT HAVE_STRDUP)
    message(WARNING "The strdup was NOT found")
endif()

check_cxx_symbol_exists(strerror "string.h" HAVE_STRERROR)
if(NOT HAVE_STRERROR)
    message(WARNING "The strerror was NOT found")
endif()

check_cxx_symbol_exists(strncasecmp "strings.h" HAVE_STRNCASECMP)
if(NOT HAVE_STRNCASECMP)
    message(WARNING "The strncasecmp was NOT found")
endif()

check_symbol_exists(strstr "string.h" HAVE_STRSTR)
if(NOT HAVE_STRSTR)
    message(WARNING "The strstr was NOT found")
endif()

check_cxx_symbol_exists(strtol "stdlib.h" HAVE_STRTOL)
if(NOT HAVE_STRTOL)
    message(WARNING "The strtol was NOT found")
endif()

check_cxx_symbol_exists(strtoul "stdlib.h" HAVE_STRTOUL)
if(NOT HAVE_STRTOUL)
    message(WARNING "The strtoul was NOT found")
endif()

check_cxx_symbol_exists(tzset "time.h" HAVE_TZSET)
if(NOT HAVE_TZSET)
    message(WARNING "The tzset was NOT found")
endif()

check_cxx_symbol_exists(unsetenv "stdlib.h" HAVE_UNSETENV)
if(NOT HAVE_UNSETENV)
    message(WARNING "The unsetenv was NOT found")
endif()

check_cxx_symbol_exists(usleep "unistd.h" HAVE_USLEEP)
if(NOT HAVE_USLEEP)
    message(WARNING "The usleep was NOT found")
endif()

check_cxx_symbol_exists(utime "utime.h" HAVE_UTIME)
if(NOT HAVE_UTIME)
    message(WARNING "The utime was NOT found")
endif()

check_cxx_symbol_exists(utimes "sys/time.h" HAVE_UTIMES)
if(NOT HAVE_UTIMES)
    message(WARNING "The utimes was NOT found")
endif()


if(tcmalloc AND jemalloc)
    message(FATAL_ERROR "Cannot use both, tcmalloc and jemalloc!")
endif()

set(have_tcmalloc FALSE)
if(tcmalloc)
    pkg_check_modules(TCMALLOC REQUIRED "libtcmalloc_minimal")
    if(TCMALLOC_FOUND)
        check_library_exists(tcmalloc_minimal malloc "" have_tcmalloc)
        if(have_tcmalloc)
            set(TCMALLOC_CFLAGS)
            set(TCMALLOC_LIBS "-l${TCMALLOC_LIBRARIES}")  # ${TCMALLOC_LIBRARIES} evaluates to tcmalloc_minimal
        else()
            message(WARNING "ARIA2_DEP_NOT_MET([tcmalloc_minimal]). Install it with: sudo apt get install libgoogle-perftools-dev")
        endif()
    endif()
endif()

set(have_jemalloc FALSE)
if(jemalloc)
    pkg_check_modules(JEMALLOC REQUIRED jemalloc)
    if(NOT JEMALLOC_FOUND)
        check_library_exists(jemalloc malloc "" have_jemalloc)
        if(have_jemalloc)
            set(JEMALLOC_CFLAGS)
            set(JEMALLOC_LIBS "-l${JEMALLOC_LIBRARIES}")  # ${JEMALLOC_LIBRARIES} evaluates to jemalloc
        else()
            message(WARNING "ARIA2_DEP_NOT_MET([jemalloc (unprefixed)]). Install it with: sudo apt-get install libjemalloc-dev")
        endif()
    endif()
endif()

set(have_epoll FALSE)
if(epoll)
    check_symbol_exists(epoll_create "sys/epoll.h" have_epoll)
    if(have_epoll)
        add_compile_definitions(HAVE_EPOLL=1)
    endif()
endif()

if(have_epoll)
    set(HAVE_EPOLL TRUE CACHE BOOL "epoll exists" FORCE)
else()
    set(HAVE_EPOLL FALSE CACHE BOOL "epoll NOT exists" FORCE)
endif()

check_symbol_exists(posix_fallocate "fcntl.h" have_posix_fallocate)
if(NOT have_posix_fallocate)
   message(WARNING "posix_fallocate was NOT found")
endif()

check_c_source_compiles("
    #include <fcntl.h>
    int main() {
        fallocate(2, 0, 0, 4096);
        return 0;
    }" have_fallocate
)

if(have_fallocate)
    add_compile_definitions(HAVE_FALLOCATE=1)
endif()

if(have_posix_fallocate OR have_fallocate OR have_osx OR win_build)
   add_compile_definitions(HAVE_SOME_FALLOCATE=1)
   set(HAVE_SOME_FALLOCATE TRUE CACHE BOOL "*_fallocate exists" FORCE)
endif()

set(save_CPPFLAGS "${CPPFLAGS}")
string(APPEND CPPFLAGS " ${EXTRACPPFLAGS}")
message("Checking for asctime_r")
check_cxx_source_compiles("
    #include <time.h>
    struct tm r;
    char *c;

    int main() {
        char *res = asctime_r(&r, c);
        return 0;
    }" have_asctime_r
)

if(have_asctime_r)
    message("Have asctime_r function")
    add_compile_definitions(HAVE_ASCTIME_R=1)
    set(HAVE_ASCTIME_R TRUE CACHE BOOL "Have asctime_r function")
else()
    message("NOT have asctime_r function")
    set(HAVE_ASCTIME_R FALSE CACHE BOOL "NOT have asctime_r function")
endif()

message("Checking for localtime_r")
check_cxx_source_compiles("
    #include <time.h>
    time_t t;
    struct tm r;

    int main() {
        localtime_r(&t, &r);
        return 0;
    }" have_localtime_r)

if(have_localtime_r)
    message("Have localtime_r function")
    add_compile_definitions(HAVE_LOCALTIME_R=1)
    set(HAVE_LOCALTIME_R TRUE CACHE BOOL "Have localtime_r function")
else()
    message("NOT have localtime_r function")
    set(HAVE_LOCALTIME_R FALSE CACHE BOOL "NOT have localtime_r function")
endif()

set(CPPFLAGS "${save_CPPFLAGS}")

check_symbol_exists(basename "libgen.h" BASENAME_EXISTS)
if(BASENAME_EXISTS)
   set(HAVE_BASENAME TRUE CACHE BOOL "The basename has been found" FORCE)
else()
    set(HAVE_BASENAME FALSE CACHE BOOL "The basename has NOT been found" FORCE)
endif()

check_symbol_exists(gai_strerror "netdb.h" GAI_STRERROR_EXISTS)
if(GAI_STRERROR_EXISTS)
   set(HAVE_GAI_STRERROR TRUE CACHE BOOL "The gai has been found" FORCE)
else()
    set(HAVE_GAI_STRERROR FALSE CACHE BOOL "The gai has NOT been found" FORCE)
endif()

check_symbol_exists(getaddrinfo "netdb.h" GETADDRINFO_EXISTS)
if(GETADDRINFO_EXISTS)
   set(HAVE_GETADDRINFO TRUE CACHE BOOL "The getaddrinfo has been found" FORCE)
else()
    set(HAVE_GETADDRINFO FALSE CACHE BOOL "The getaddrinfo has NOT been found" FORCE)
endif()

check_symbol_exists(gettimeofday "sys/time.h" GETTIMEOFDAY_EXISTS)
if(HAVE_GETTIMEOFDAY)
   set(HAVE_GETTIMEOFDAY TRUE CACHE BOOL "The gettimeofday is present" FORCE)
else()
    set(HAVE_GETTIMEOFDAY FALSE CACHE BOOL "The gettimeofday is NOT present" FORCE)
endif()

set(localStrptimeLocation ${CMAKE_CURRENT_SOURCE_DIR}/src)
check_symbol_exists(strptime "${localStrptimeLocation}/strptime.h" STRPTIME_EXISTS)
if(STRPTIME_EXISTS)
   set(HAVE_STRPTIME TRUE CACHE BOOL "The strptime is present" FORCE)
else()
    set(HAVE_STRPTIME FALSE CACHE BOOL "The strptime is NOT present" FORCE)
endif()

check_symbol_exists(daemon unistd.h DAEMON_EXISTS)
if(DAEMON_EXISTS)
   message("The daemon is present")
   set(HAVE_DAEMON TRUE CACHE BOOL "The daemon is present" FORCE)
else()
    set(HAVE_DAEMON FALSE CACHE BOOL "The daemon is NOT present" FORCE)
endif()

check_symbol_exists(poll "poll.h" POLL_EXISTS)  # at configure.ac, the variable to hold the result is: have_poll
if(POLL_EXISTS)
    set(HAVE_POLL TRUE CACHE BOOL "The poll is present" FORCE)
else()
    set(HAVE_POLL FALSE CACHE BOOL "The poll is NOT present" FORCE)
endif()

if(MINGW)
    set(MINGW_BUILD TRUE CACHE BOOL "Build for Mingw" FORCE)
    set(HAVE_GETADDRINFO TRUE CACHE BOOL "Enable getaddrinfo" FORCE)
    set(HAVE_GAI_STRERROR FALSE CACHE BOOL "Disable getaddrinfo" FORCE)
else()
    set(MINGW_BUILD FALSE CACHE BOOL "Not build for Mingw" FORCE)
endif()

check_cxx_symbol_exists(timegm "time.h" TIMEGM_EXISTS)
if(TIMEGM_EXISTS)
    set(HAVE_TIMEGM TRUE CACHE BOOL "The timegm has been found" FORCE)
else()
    set(HAVE_TIMEGM FALSE CACHE BOOL "The timegm has NOT been found" FORCE)
endif()

# TODOm: this library cannot be found, yet
check_cxx_symbol_exists(port_associate "port.h" PORT_ASSOCIATE_EXISTS)
if(PORT_ASSOCIATE_EXISTS)
    set(HAVE_PORT_ASSOCIATE TRUE CACHE BOOL "The port_associate has been found" FORCE)
else()
    set(HAVE_PORT_ASSOCIATE FALSE CACHE BOOL "The port_associate has NOT been found" FORCE)
endif()

set(save_CMAKE_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
set(save_CMAKE_INCLUDE_DIRS "${CMAKE_INCLUDE_DIRS}")

pkg_check_modules(KQUEUE "libkqueue")
string(APPEND CMAKE_REQUIRED_LIBRARIES " ${KQUEUE_LIBRARIES}")   # KQUEUE_LIBRARIES evaluates to: kqueue
string(APPEND CMAKE_REQUIRED_INCLUDES " ${KQUEUE_INCLUDE_DIRS}") # KQUEUE_INCLUDE_DIRS evaluates to: /usr/include/kqueue

check_symbol_exists(kqueue "sys/event.h" KQUEUE_EXISTS)
if(KQUEUE_EXISTS)
    set(have_kqueue TRUE)
else()
    check_function_exists(kqueue KQUEUE_EXISTS_WITHOUT_DECL)
    if(KQUEUE_EXISTS_WITHOUT_DECL)
        message(WARNING "Function is availabe at kqueue library, but it's declaration is NOT provided into event.h header")
        set(have_kqueue TRUE)
    endif()
endif()

if(have_kqueue)
    set(HAVE_KQUEUE TRUE CACHE BOOL "The kqueue has been found." FORCE)
    set(keventCode [==[
            #include <sys/time.h>
            #include <sys/types.h>
            #include <sys/event.h>

            struct kevent event;

            int main() {
                event.udata = reinterpret_cast<intptr_t>(&event);
                return 0;
            }
        ]==]
    )

    message("Whether struct kevent.udata is intptr_t")
    check_cxx_source_compiles("${keventCode}" kevent_udata_intptr_t)
    if(kevent_udata_intptr_t)
        add_compile_definitions(KEVENT_UDATA_INTPTR_T=1)
        message("Define to 1 if struct kevent.udata is intptr_t")
    endif()
else()
	message(WARNING "The kqueue has NOT been found")
#    message("Try installing it with the command: sudo apt install libkqueue-dev")
#    message("Possible location where it might be located is: /usr/include/kqueue/sys/event.h")
#    message("and not at the /usr/include. So I have to include the /usr/include/kqueue/ path in")
#    message("in order to find the header  (event.h)")
endif()

set(CMAKE_REQUIRED_LIBRARIES "${save_CMAKE_LIBRARIES}")
set(CMAKE_INCLUDE_DIRS "${save_CMAKE_INCLUDE_DIRS}")

# HINT: this is basically in BSD and macOS operating systems
check_struct_has_member(
    "struct sockaddr_in"
    sin_len
    "sys/types.h; sys/socket.h; netinet/in.h"
    HAVE_SIN_LEN
    LANGUAGE CXX
)

if(HAVE_SIN_LEN)
    add_compile_definitions(HAVE_SOCKADDR_IN_SIN_LEN=1)
    message("Define to 1 if struct sockaddr_in has sin_len member.")
endif()

# HINT: this is basically in BSD and macOS operating systems
check_struct_has_member(
    "struct sockaddr_in6"
    sin6_len
    "sys/types.h; sys/socket.h; netinet/in.h"
    HAVE_SIN6_LEN
    LANGUAGE CXX
)

if(HAVE_SIN6_LEN)
    add_compile_definitions(HAVE_SOCKADDR_IN6_SIN6_LEN=1)
    message("Define to 1 if struct sockaddr_in6 has sin6_len member.")
endif()

message("Whether struct option.name is assignable from const char*")
set(nameAssignableFromConst [==[
        #include <unistd.h>
        #include <getopt.h>

        int main() {
            const char* s = "const char";
            option op;
            op.name = s;
            return 0;
        }
    ]==]
)

check_cxx_source_compiles("${nameAssignableFromConst}" have_option_const_name)
if(have_option_const_name)
    add_compile_definitions(HAVE_OPTION_CONST_NAME=1)
    message("Define 1 if struct option.name is const char*")
endif()

if(websocket)
    add_subdirectory(deps/wslay)
    add_compile_definitions(ENABLE_WEBSOCKET=1)
    message("Define 1 if WebSocket support is enabled")

    # TODO:  Should check in the future as currently this is not enabled.
    string(APPEND WSLAY_CFLAGS " ${CMAKE_CURRENT_BINARY_DIR}/deps/wslay/lib/includes ${CMAKE_CURRENT_LIST_DIR}/deps/wslay/lib/includes")
    string(APPEND WSLAY_LIBS " ${CMAKE_CURRENT_BINARY_DIR}/deps/wslay/lib/libwslay.la")  # TODOm: check if this is needed anyway because it uses libwslay.la
endif()

if(websocket)
    set(ENABLE_WEBSOCKET TRUE CACHE BOOL "Websocket is enabled" FORCE)
endif()

if(libaria2)
    set(ENABLE_LIBARIA2 TRUE CACHE BOOL "Libaria2 is enabled" FORCE)
endif()

# AC_SUBST([bashcompletiondir]) # TODO: uses the AM_CONDITIONAL actually uses AC_SUBST but set it like this for searching purposes in order to be able to find it

if(ARIA2_STATIC)
    string(APPEND EXTRALDFLAGS " -all-static")
    if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL MSVC OR NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Mingw" OR NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        string(APPEND EXTRALIBS " -lpthread -ldl -lrt")
    endif()
endif()

if(win_build AND libaria2)
    # Creating dll needs this
    string(APPEND EXTRALDFLAGS " -no-undefined")
endif()
# TODO: the respective substitution should be done with cofigure_file() command
# AC_SUBST([EXTRACFLAGS])
# AC_SUBST([EXTRACXXFLAGS])
# AC_SUBST([EXTRACPPFLAGS])
# AC_SUBST([EXTRALDFLAGS])
# AC_SUBST([EXTRALIBS])

set(save_CXXFLAGS "${CXXFLAGS}")
# I should check this, with foreach() loop, in the test project I have
if(werror)
    include(CheckCompilerFlags)
    check_compiler_flag(CXX -Wall HAVE_WALL_FLAG)
    check_compiler_flag(CXX -Werror HAVE_WERROR_FLAG)
    check_compiler_flag(CXX -Wformat-security HAVE_FORMAT_SECURITY)

    if(HAVE_WALL_FLAG)
        string(APPEND CXXFLAGS " -Wall")
    endif()

    if(HAVE_WERROR_FLAG)
        string(APPEND CXXFLAGS " -Werror")
    endif()

    if(HAVE_FORMAT_SECURITY)
        string(APPEND CXXFLAGS " -Wformat-security")
    endif()

endif()

string(APPEND WARNCXXFLAGS " ${CXXFLAGS}")

set(CXXFLAGS "${save_CXXFLAGS}")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/)
message("")
message(STATUS "Summary of build options\n")

set(printKeys "Build;Host;Target;Install prefix;CC;CXX;CPP;CXXFLAGS;CFLAGS;CPPFLAGS;LDFLAGS;\
LIBS;DEFS;CXX1XCXXFLAGS;EXTRACXXFLAGS;EXTRACFLAGS;EXTRACPPFLAGS;EXTRALDFLAGS;\
EXTRALIBS;WARNCXXFLAGS;LibUV;SQLite3;SSL Support;AppleTLS;WinTLS;GnuTLS;OpenSSL;\
CA Bundle;LibNettle;LibGmp;LibGcrypt;LibXML2;LibExpat;LibCares;Zlib;Libssh2;Tcmalloc;\
Jemalloc;Epoll;Bittorrent;Metalink;XML-RPC;Message Digest;WebSocket;Libaria2;bash_completion;\
Static Build")

set(printValues "${CMAKE_HOST_SYSTEM_NAME};${CMAKE_SYSTEM_NAME};unknown_value;${CMAKE_INSTALL_PREFIX};${CC};${CXX};${CPP};${CXXFLAGS};\
${CFLAGS};${CPPFLAGS};${LDFLAGS};${LIBS};${DEFS};${CXX1XCXXFLAGS};${EXTRACXXFLAGS};${EXTRACFLAGS};\
${EXTRACPPFLAGS};${EXTRALDFLAGS};${EXTRALIBS};${WARNCXXFLAGS};${have_libuv};${have_sqlite3};${have_ssl};\
${have_appletls};${have_wintls} (LIBS=${WINTLS_LIBS});${have_libgnutls};${have_openssl};${ca_bundle};${have_libnettle};\
${have_libgmp};${have_libgcrypt};${have_libxml2};${have_libexpat};${have_libcares};${have_zlib};${have_libssh2};\
${have_tcmalloc};${have_jemalloc};${have_epoll};${enable_bittorrent};${enable_metalink};${enable_xml_rpc};${use_md};${enable_websocket};\
${ENABLE_LIBARIA2};${bashcompletiondir};${ARIA2_STATIC}")

printBuildOptions(printKeys printValues)

# string(APPEND ...)
# If multiple compiler flags need to be set, they need to be specified as a single string, not as a list.
# CMake will not properly handle flag variables if their contents contain semicolons, which is what a
# list would be turned into if specified by the project