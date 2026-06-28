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
        if(LIBNETTLE_FOUND)
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

        # if(ARIA2_STATIC)
        #     set(LIBSSH2_CFLAGS "-DLIBSSH2_API")
        # endif()
    else()
        message(WARNING "The libssh2 library is missing")
        
        if(LIBSSH2_REQUESTED)
            message(WARNING "ARIA2_DEP_NOT_MET([libssh2])")
        endif()
    endif()
endif()


set(have_libcares FALSE)
find_package(libcares2 REQUIRED)
if(libcares2_FOUND)
    set(have_libcares TRUE)
    add_compile_definitions(HAVE_LIBCARES=1)
    message(STATUS "Define to 1 if you have libcares")
else()
    message(WARNING "libcares couldn't find in the system. Try install it first")
    message(WARNING "ARIA2_DEP_NOT_METq([libcares)")
endif()

set(use_md "")
if(have_appletls)
    set(use_md "apple")
    add_compile_definitions(USE_APPLE_MD=1)
    message(STATUS "What message digest implementation to use")
elseif(have_libnettle)
    add_compile_definitions(USE_LIBNETTLE_MD=1)
    message(STATUS "What message digest implementation to use")
    set(use_md "libnettle")
elseif(have_libcrypt)
    add_compile_definitions(USE_LIBCRYPT_MD=1)
    message(STATUS "What message digest implementation to use")
    set(use_md "libcrypt")
elseif(have_openssl)
    add_compile_definitions(USE_OPENSSL_MD=1)
    message(STATUS "What message digest implementation to use")
    set(use_md "openssl")
else()
    add_compile_definitions(USE_INTERNAL_MD=1)
    message(STATUS "What message digest implementation to use")
    set(use_md "internal")
endif()

if(have_ssl)
    add_compile_definitions(ENABLE_SSL=1)
    message("Define to 1 if ssl support is enabled")
    set(ENABLE_SSL true)
else()
    set(have_ssl no)
    set(ENABLE_SSL false)
endif()

# this can be done more clearlyl with a list of the variables that I want to check if they
# are true or false.

if(have_osx)
    set(HAVE_OSX TRUE)
endif()

if(have_appletls)
    set(HAVE_APPLETLS TRUE)
endif()

if(have_wintls)
    set(HAVE_WINTLS TRUE)
endif()

if(use_md STREQUAL "apple")
    set(USE_APPLE_MD TRUE)
elseif(use_md STREQUAL "windows")
    set(USE_WINDOWS_MD TRUE)
elseif(use_md STREQUAL "libnettle")
    set(USE_LIBNETTLE_MD TRUE)
elseif(use_md STREQUAL "libgcrypt")
    set(USE_LIBGCRYPT TRUE)
elseif(use_md STREQUAL "openssl")
    set(USE_OPENSSL_MD TRUE)
elseif(use_md STREQUAL "internal")
    set(USE_INTERNAL_MD TRUE)
endif()

if(have_libgnutls)
    set(HAVE_LIBGNUTLS TRUE)
endif()

if(have_libnettle)
    set(HAVE_LIBNETTLE TRUE)
endif()

if(have_libgmp)
    set(HAVE_LIBGMP TRUE)
endif()

if(libgcrypt)
    set(HAVE_LIBGCRYPT TRUE)
endif()

if(have_openssl)
    set(HAVE_OPENSSL TRUE)
endif()


if(have_libgmp OR have_libgcrypt OR have_openssl)
    set(USE_INTERNAL_BIGNUM FALSE)
else()
    add_compile_definitions(USE_INTERNAL_BIGNUM=1)
    message("Define to 1 if the internal BIGNUM support is enabled")
    set(USE_INTERNAL_BIGNUM TRUE)
endif()

if(have_libnettle OR have_libgcrypt OR have_openssl)
    set(USE_INTERNAL_ARC4 FALSE)
else()
    add_compile_definitions(USE_INTERNAL_ARC4=1)
    set(USE_INTERNAL_ARC4 TRUE)
endif()

if(enable_bittorrent)
    add_compile_definitions(ENABLE_BITTORRENT=1)
    message("Define to 1 if BitTorrent support is enabled")
    set(ENABLE_BITTORRENT TRUE)
else()
    if(bittorrent)
        message(WARNING "ARIA2_FET_NOT_SUPPORTED[bittorrent")
    endif()
    set(enable_bittorrent FALSE)
    set(ENABLE_BITTORRENT FALSE)
endif()

if((have_libxml2 OR have_libexpat) AND enable_metalink)
    add_compile_definitions(ENABLE_METALINK=1)
    message("Define to 1 if Metalink support is enabled")
    set(ENABLE_METALINK TRUE)
else()
    if(metalink)
        message(WARNING "ARIA2_FET_NOT_SUPPORTED[metalink]")
    endif()
    set(enable_metalink FALSE)
    set(ENABLE_METALINK FALSE)
endif()

if(have_libxml2 OR have_libexpat)
    set(HAVE_SOME_XMLLIB)
endif()

if(have_libxml2 OR have_libexpat)
    set(enable_xml_rpc TRUE)
else()
    set(enable_xml_rpc FALSE)
endif()

if(enable_xml_rpc)
    add_compile_definitions(ENABLE_XML_RPC=1)
    message("Define to 1 if XML-RPC support is enabled.")
endif()

if(enable_xml_rpc)
    set(ENABLE_XML_RPC TRUE)
endif()

if(have_libxml2)
    set(HAVE_LIBXML2 TRUE)
endif()

if(have_libexpat)
    set(HAVE_LIBEXPAT TRUE)
endif()

if(have_libcares)
    add_compile_definitions(ENABLE_ASYNC_DNS=1)
    message("Define to 1 asynchronous DNS support is enabled")
    set(ENABLE_ASYNC_DNS TRUE)
else()
    set(ENABLE_ASYNC_DNS FALSE)
endif()

if(have_zlib)
    set(HAVE_ZLIB TRUE)
endif()

if(have_sqlite3)
    set(HAVE_SQLITE3 TRUE)
endif()

if(have_libssh2)
    set(HAVE_LIBSSH2 TRUE)
endif()

if(CMAKE_HOST_SOLARIS)
    #
    #
    #
endif()

check_include_files("argz.h; arpa/inet.h; fcntl.h; float.h; inttypes.h; langinfo.h; libintl.h; limits.h; locale.h; malloc.h; netdb.h; netinet/in.h; netinet/tcp.h; poll.h; port.h; signal.h; stddef.h; stdint.h; stdio_ext.h; stdlib.h; string.h; 
                     strings.h; sys/ioctl.h; sys/param.h; sys/resource.h; sys/signal.h; sys/socket.h; sys/time.h; sys/types.h; sys/uio.h; sys/utsname.h; termios.h; unistd.h; utime.h; wchar.h; ifaddrs.h; pwd.h;"
                     HAVE_HEADER_FILES)



# from line 720 of configure.ac file

include(CheckTypeSize)

# in order to be able to find the c struct, I have to enable C language
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD_REQUIRED TRUE)
# now it will be able to find the c struct
include(CheckStructHasMember)
check_struct_has_member(
    "struct timespec"
    tv_sec
    time.h
    have_timespec
)
if(have_timespec)
    add_compile_definitions(HAVE_A2_STRUCT_TIMESPEC=1)
    message("Define to 1 if the system has the type 'struct timespec'")
endif()


find_package(Gettext 0.19 REQUIRED)  # apt-get install gettext, if doesn't exist in the system
if(Gettext_FOUND)
    message(STATUS "The gettext was found")
endif()

include(CheckSymbolExists)
include(CheckCXXSymbolExists)
check_cxx_symbol_exists(error_at_line "error.h" HAVE_ERROR_AT_LINE)
if(HAVE_ERROR_AT_LINE)
    message("The error_at_line function was found")
endif()

check_cxx_symbol_exists(memcmp "cstring" HAVE_MEMCMP)
if(HAVE_MEMCMP)
    message("The memcmp function was found")
endif()

check_cxx_symbol_exists(mktime "ctime" HAVE_MKTIME)
if(HAVE_MKTIME)
    message(STATUS "The mktime was found")
endif()

check_cxx_symbol_exists(stat "sys/stat.h" HAVE_STAT)
if(HAVE_STAT)
    message(STATUS "The stat was found")
endif()

check_cxx_symbol_exists(strftime "ctime" HAVE_STRFTIME)
if(HAVE_STRFTIME)
    message(STATUS "The strftime was found")
endif()

check_cxx_symbol_exists(vprintf cstdio HAVE_VPRINTF )
if(HAVE_VPRINTF)
    message(STATUS "The vrpintf was found")
endif()

check_cxx_symbol_exists(fork unistd.h HAVE_FORK)
if(HAVE_FORK)
    message(STATUS "The fork has been found")
endif()

check_cxx_symbol_exists(strtod stdlib.h HAVE_STRTOD)
if(HAVE_STRTOD)
    message(STATUS "The strtod has been found")
endif()


# From the line 772 AC_CHECK_FUNCS()
check_symbol_exists(__argz_count "argz.h" HAVE_ARGZ_COUNT)
if(HAVE_ARGZ_COUNT)
    message(STATUS "The argz_count function has been found")
endif()

check_cxx_symbol_exists(__argz_count "argz.h" HAVE_ARGZ_COUNT)
if(HAVE_ARGZ_COUNT)
    message("The argz has been found")
endif()

check_cxx_symbol_exists(__argz_next "argz.h" HAVE_ARGZ_NEXT)
if(HAVE_ARGZ_NEXT)
    message("The argz_next has been found")
endif()

check_cxx_symbol_exists(__argz_stringify "argz.h" HAVE_STRINGIFY)
if(HAVE_STRINGIFY)
    message("The __argz_stringify has been found")
endif()

check_cxx_symbol_exists(atexit stdlib.h HAVE_ATEXIT)
if(HAVE_ATEXIT)
    message("The atexit has been found")
endif()

check_cxx_symbol_exists(ftruncate unistd.h HAVE_FTRUNCATE)
if(HAVE_FTRUNCATE)
    message("the ftruncate has been found")
endif()

check_cxx_symbol_exists(getcwd unistd.h HAVE_GETCWD)
if(HAVE_GETWD)
    message("The getcwd has been found")
endif()

check_cxx_symbol_exists(getentropy unistd.h HAVE_ENTROPY)
if(HAVE_ENTROPY)
    message("The getentropy has been found")
endif()

check_cxx_symbol_exists(gethostbyaddr netdb.h HAVE_HOSTBYADDR)
if(HAVE_HOSTBYADDR)
    message("The gethostbyaddr has been found")
endif()

check_cxx_symbol_exists(gethostbyname netdb.h HAVE_HOSTBYNAME)
if(HAVE_HOSTBYNAME)
    message("The gethostbyname has been found")
endif()

check_cxx_symbol_exists(getifaddrs ifaddrs.h HAVE_IFADDRS)
if(HAVE_IFADDRS)
    message("The getifaddrs has been found")
endif()

check_cxx_symbol_exists(getpagesize unistd.h HAVE_PAGESIZE)
if(HAVE_PAGESIZE)
    message("The getpagesize function has been found")
endif()

check_symbol_exists(memchr string.h HAVE_MEMCHR)
if(HAVE_MEMCHR)
    message("The memchr has been found")
endif()

check_cxx_symbol_exists(memmove string.h HAVE_MEMMOVE)
if(HAVE_MEMMOVE)
    message("The memmove has been found")
endif()

check_cxx_symbol_exists(mempcpy string.h HAVE_MEMPCPY)
if(HAVE_MEMPCPY)
    message("The mempcpy has been found")
endif()

check_cxx_symbol_exists(memset string.h HAVE_MEMSET)
if(HAVE_MEMSET)
    message("The memset has been found")
endif()

check_cxx_symbol_exists(mkdir "sys/stat.h" HAVE_MKDIR)
if(HAVE_MKDIR)
    message("The mkdir has been found")
endif()

check_cxx_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
if(HAVE_MMAP)
    message("The mmap has been found")
endif()

check_cxx_symbol_exists(munmap "sys/mman.h" HAVE_MUNMAP)
if(HAVE_MUNMAP)
    message("The munmap has been found")
endif()

check_cxx_symbol_exists(nl_langinfo langinfo.h HAVE_LANGINFO)
if(HAVE_LANGINFO)
    message("The nl_langinfo has been found")
endif()

check_cxx_symbol_exists(posix_fadvise fcntl.h HAVE_POSIXFADVISE)
if(HAVE_POSIXFADVISE)
    message("The posix_fadvise has been found")
endif()

check_cxx_symbol_exists(posix_memalign stdlib.h HAVE_POSIXMEMALIGN)
if(HAVE_POSIXMEMALIGN)
    message("The posix_memalign has been found")
endif()

check_cxx_symbol_exists(pow "tgmath.h" HAVE_POW)
if(HAVE_POW)
    message("The pow has been found")
endif()

check_cxx_symbol_exists(putenv stdlib.h HAVE_PUTENV)
if(HAVE_PUTENV)
    message("The putenv has been found")
endif()

check_cxx_symbol_exists(rmdir unistd.h HAVE_RMDIR)
if(HAVE_RMDIR)
    message("The rmdir has been found")
endif()

check_cxx_symbol_exists(select "sys/select.h" HAVE_SELECT)
if(HAVE_SELECT)
    message("The select has been found")
endif()

check_cxx_symbol_exists(setlocale locale.h HAVE_LOCALE)
if(HAVE_LOCALE)
    message("The setlocale has been found")
endif()

check_cxx_symbol_exists(sigaction signal.h HAVE_SIGACTION)
if(HAVE_SIGACTION)
    message("The sigaction has been found")
endif()

check_cxx_symbol_exists(sleep unistd.h HAVE_SLEEP)
if(HAVE_SLEEP)
    message("The sleep has been found")
endif()

check_cxx_symbol_exists(socket "sys/socket.h" HAVE_SOCKET)
if(HAVE_SOCKET)
    message("The socket has been found")
endif()

check_cxx_symbol_exists(stpcpy string.h HAVE_STPCPY)
if(HAVE_STPCPY)
    message("The stpcpy has been found")
endif()

check_cxx_symbol_exists(strcasecmp strings.h HAVE_STRCASECMP)
if(HAVE_STRCASECMP)
    message("The strcasecmp has been found")
endif()

check_symbol_exists(strchr string.h HAVE_STRCHR)
if(HAVE_STRCHR)
    message("The strchr has been found")
endif()

check_cxx_symbol_exists(strcspn string.h HAVE_STRCSPN)
if(HAVE_STRCSPN)
    message("The strcspn has been found")
endif()

check_cxx_symbol_exists(strdup string.h HAVE_STRDUP)
if(HAVE_STRDUP)
    message("The strdup has been found")
endif()

check_cxx_symbol_exists(strerror string.h HAVE_STRERROR)
if(HAVE_STRERROR)
    message("The strerror has been found")
endif()

check_cxx_symbol_exists(strncasecmp strings.h HAVE_STRNCASECMP)
if(HAVE_STRNCASECMP)
    message("The strncasecmp has been found")
endif()

check_symbol_exists(strstr string.h HAVE_STRSTR)
if(HAVE_STRSTR)
    message("The strstr has been found")
endif()

check_cxx_symbol_exists(strtol stdlib.h HAVE_STRTOL)
if(HAVE_STRTOL)
    message("The strtol has been found")
endif()

check_cxx_symbol_exists(strtoul stdlib.h HAVE_STRTOUL)
if(HAVE_STRTOUL)
    message("The strtoul has been found")
endif()

check_cxx_symbol_exists(tzset time.h HAVE_TZSET)
if(HAVE_TZSET)
    message("The tzset has been found")
endif()

check_cxx_symbol_exists(unsetenv stdlib.h HAVE_UNSETENV)
if(HAVE_UNSETENV)
    message("The unsetenv has been found")
endif()

check_cxx_symbol_exists(usleep unistd.h HAVE_USLEEP)
if(HAVE_USLEEP)
    message("The usleep has been found")
endif()

check_cxx_symbol_exists(utime utime.h HAVE_UTIME)
if(HAVE_UTIME)
    message("The utime has been found")
endif()

check_cxx_symbol_exists(utimes "sys/time.h" HAVE_UTIMES)
if(HAVE_UTIMES)
    message("The utimes has been found")
endif()


if(tcmalloc AND jemalloc)
    message(ERROR "Cannot use both, tcmalloc and jemalloc!")
endif()

set(have_tcmalloc FALSE)
if(tcmalloc)
    pkg_check_modules(TCMALLOC libtcmalloc REQUIRED)
    if(NOT TCMALLOC_FOUND)
        check_library_exists(tcmalloc_minimal malloc "" have_tcmalloc)
        if(have_tcmalloc)
            # some commands here
            # ...
            # ...
        else()
            if(have_tcmalloc)
                message(WARNING "ARIA2_DEP_NOT_MET: tc_malloc_minimal")
            endif()
        endif()
    endif()
endif()

set(have_jmalloc FALSE)
if(jemalloc)
    pkg_check_modules(JEMALLOC jemalloc REQUIRED)
    if(NOT JEMALLOC_FOUND)
        check_library_exists(jemalloc malloc "" have_jemalloc)
        if(have_jemalloc)
            # some commands here
            # ...
            # ...
        else()
            if(jemalloc)
                message(WARNING "ARIA2_DEP_NOT_MET")
            endif()
        endif()
    endif()
endif()

set(have_epoll FALSE)
if(epoll)
    check_symbol_exists(epoll_create "sys/epoll.h" have_epoll)
    if(have_epoll)
        add_compile_definitions(HAVE_EPOLL=1)
        message(STATUS "Define to 1 if epoll is available")
    endif()
endif()

check_symbol_exists(posix_fallocate "fcntl.h" HAVE_POSIX_FALLOCATE)
if(HAVE_POSIX_FALLOCATE)
   message("The fallocate has been found")
endif()

if(HAVE_POSIX_FALLOCATE OR HAVE_FALLOCATE OR HAVE_OSX OR WIN_BUILD)
   add_compile_definitions(HAVE_SOME_FALLOCATE=1)
   message(STATUS "Define to 1 if *_fallocate is available")
endif()

# need one more with the AM_CONDITIONAL line 890

check_symbol_exists(basename libgen.h HAVE_BASENAME)
if(HAVE_BASENAME)
   message("The basename has been found")
endif()

check_symbol_exists(gai_strerror "netdb.h" HAVE_GAI_STRERROR)
if(HAVE_GAI_STRERROR)
   message("The gai has been found")
endif()

check_symbol_exists(getaddrinfo "netdb.h" HAVE_GETADDRINFO)
if(HAVE_GETADDRINFO)
   message("the getaddrinfo has been found")
endif()

check_symbol_exists(gettimeofday "sys/time.h" HAVE_GETTIMEOFDAY)
if(HAVE_GETTIMEOFDAY)
   message("the gettimeofday is present")
endif()

check_symbol_exists(strptime "time.h" HAVE_STRPTIME)
if(HAVE_STRPTIME)
   message("The strptime is present")
endif()

check_symbol_exists(daemon unistd.h HAVE_DAEMON)
if(HAVE_DAEMON)
   message("The daemon is present")
endif()
# TODO: missing the header file to find the pol function
check_symbol_exists(pol have_poll)
if(have_poll)
    # write into the config.h.in file the HAVE_POLL=true
    # the respective of AM_CONDITIONAL([HAVE_POLL], [test "x$have_poll" = "yes"])
endif()

if(MINGW)
    # set the AM_CONDITIONALS in the config.h.in
endif()

check_cxx_symbol_exists(timegm time.h HAVE_TIMEGM)
if(HAVE_TIMEGM)
    message("The timegm has been found")
endif()

# TODO: this library cannot be found, yet
check_cxx_symbol_exists(port_associate port.h HAVE_PORT_ASSOCIATE)
if(HAVE_PORT_ASSOCIATE)
    message("The port_associate has been found")
endif()

#==========================#
set(oldIncludeHeaders "${CMAKE_REQUIRED_INCLUDES}")

set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES} /usr/include/kqueue")
check_cxx_symbol_exists(kqueue "sys/event.h" HAVE_KQUEUE)
if(HAVE_KQUEUE)
    message("The kqueue has been found.")
else()
    message("Try installing it with the command: sudo apt install libkqueue-dev")
    message("Possible location where it might be located is: /usr/include/kqueue/sys/event.h")
    message("and not at the /usr/include. So I have to include the /usr/include/kqueue/ path in")
    message("in order to find the header  (event.h)")
endif()


include(CheckCXXSourceCompiles)
set(keventCode [==[
        #include <sys/time.h>
        #include <sys/types.h>
        #include <sys/event.h>

        struct kevent event;

        int main() {
            event.udata = reinterpret_cast<intptr_r>(&event);
            return 0;
        }
    ]==]
)

if(HAVE_KQUEUE)
    message(STATUS "Whether struct kevent.udata is intrptr_t")
    check_cxx_source_compiles("${keventCode}" kevent_udata_intptr_t)
    if(kevent_udata_intptr_t)
      add_compile_definitions(KEVENT_UDATA_INTPTR_T=1)
        message(STATUS "Define to 1 if struct kevent.udata is intptr_t")
    endif()
endif()

set(CMAKE_REQUIRED_INCLUDES "${oldIncludeHeaders}")
#==========================#

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

message(STATUS "Whether struct option.name is assignable from const char*")

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
    message(STATUS "The option.name is assignable from const char*")
    add_compile_definitions(HAVE_OPTION_CONST_NAME=1)
    message(STATUS "Define 1 if struct option.name is const char*")
endif()


if(websocket)
    add_subdirectory(deps/wslay)
    add_compile_definitions(ENABLE_WEBSOCKET=1)
    message(STATUS "Define 1 if WebSocket support is enabled")

    set(WSLAY_CFLAGS "${CMAKE_CURRENT_BINARY_DIR}/deps/wslay/lib/includes" "${CMAKE_CURRENT_LIST_DIR}/deps/wslay/lib/includes")
    # TODO:  Should check in the future as currently this is not enabled.
    set(WSLAY_LIBS "${CMAKE_CURRENT_BINARY_DIR}/deps/wslay/lib/libwslay.la")
endif()

if(websocket)
    # set the ENABLED_WEBSOCKET with the configure_file() cmake command
endif()

if(libaria2)
    # set the ENABLE_LIBARIA2  with the configure_file() cmake command
endif()

# line 1051 
# AC_SUBST([bashcompletiondir]) check this again


