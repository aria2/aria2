cmake_minimum_required(VERSION 3.15)

project(
  aria2 
  VERSION 1.36.0 
  HOMEPAGE_URL https://aria2.github.io/
  LANGUAGES CXX C
)

# include(ConfigureChecks.cmake)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

# ####### Set compiler flags ########
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_EXTENSIONS OFF)

include(GenerateConfigH)

config_h_add_compile_definitions(PACKAGE="${PROJECT_NAME}")
config_h_add_compile_definitions(PACKAGE_NAME="${PROJECT_NAME}")
config_h_add_compile_definitions(PACKAGE_STRING="${PROJECT_NAME} ${PROJECT_VERSION}")
config_h_add_compile_definitions(PACKAGE_TARNAME="${PROJECT_NAME}")
config_h_add_compile_definitions(PACKAGE_URL="${PROJECT_HOMEPAGE_URL}")
config_h_add_compile_definitions(PACKAGE_BUGREPORT="https://github.com/aria2/aria2/issues")
config_h_add_compile_definitions(PACKAGE_VERSION="${PROJECT_VERSION}")
config_h_add_compile_definitions(VERSION="${PROJECT_VERSION}")

set(BUILD_ID "${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_HOST_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}")
set(HOST_ID "${CMAKE_HOST_SYSTEM_PROCESSOR}-${CMAKE_HOST_SYSTEM_NAME}")
set(TARGET_ID "${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}")

config_h_add_compile_definitions(HOST="${HOST_ID}")
config_h_add_compile_definitions(BUILD="${BUILD_ID}")
config_h_add_compile_definitions(TARGET="${TARGET_ID}")


if(WIN32)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_POSIX_C_SOURCE=1")
  
  if(MSVC)
    # set(EXTRALIBS ws2_32 wsock32 gdi32 iphlpapi psapi ${EXTRALIBS})
    list(APPEND EXTRALDFLAGS /DYNAMICBASE /NXCOMPAT)

  else() # MINGW
    list(APPEND EXTRALIBS -lws2_32 -lwsock32 -lgdi32 -liphlpapi -lpsapi ${EXTRALIBS})
    set(EXTRALDFLAGS "${EXTRALDFLAGS} -Wl,--dynamicbase -Wl,--nxcompat")
  endif()
endif()

option(WITH_LIBUV "Build with libuv" off)
option(WITH_APPLETLS "Build with appletls" on)
option(WITH_WINTLS "Build with wintls" on)
option(WITH_GNUTLS "Build with gnutls" on)
option(WITH_LIBNETTLE "Build with libnettle" on)
option(WITH_LIBGMP "Build with libgmp" on)
option(WITH_LIBGCRYPT "Build with libgcrypt" on)
option(WITH_OPENSSL "Build with openssl" on)
option(WITH_SQLITE3 "Build with SQLite3" on)
option(WITH_LIBXML2 "Build with libxml2" on)
option(WITH_LIBEXPAT "Build with libexpat" on)
option(WITH_LIBCARES "Build with libcares" on)
option(WITH_LIBZ "Build with libz" on)
option(WITH_TCMALLOC "Build with tcmalloc" off)
option(WITH_JEMALLOC "Build with jemalloc" off)
option(WITH_LIBSSH2 "Build with libssh2" on)

option(ENABLE_SSL "Build with ssl support" on)
option(ENABLE_BITTORRENT "Build with bittorrent support" on)
option(ENABLE_METALINK "Build with metalink support" on)
option(ENABLE_WEBSOCKET "Build with websocket support" on)
option(ENABLE_EPOLL "Build with epoll support" on)
option(ENABLE_LIBARIA2 "Build the libaria2 shared library" off)
option(ENABLE_WERROR "Builld with werror" off)

set(CA_BUNDLE "" CACHE FILEPATH "Define to choose default CA bundle.[Use FILE as default CA bundle.")
set(DISK_CACHE "" CACHE STRING "Define to choose default disk-cache size")
set(BASHCOMPLETIONDIR "" CACHE STRING "Directory to install bash_completion file")

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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /Zc:__STDC__")
  # add_compile_definitions("__STDC__=1")  # even with the /Zc:__STDC__ flag the compiler will not define this in C++ mode
endif()

if(WIN32 AND NOT MINGW)
  set(NO_UNIX)
  config_h_add_compile_macro("mode_t int")
  config_h_add_compile_definitions(NO_UNIX=1)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe")
endif()

option(ARIA2_STATIC "Set 'on' to build a statically linked aria2" off)

# i686-w64-mingw32-g++ 4.6 does not support override keyword. For
# those compilers, define CXX11_OVERRIDE to empty string. Otherwise
# define it as override. Use CXX11_OVERRIDE instead of override.
message(STATUS "Checking whether the C++ compiler supports `override` keyword")
set(_tmp_src
  "\
struct Base {
virtual void f() = 0;
};
struct Derived : Base {
virtual void f() override {}
};

int main(int argc, char* argv[]) {
  Derived x;
}
")
try_compile(CXX11_OVERRIDE
  SOURCE_FROM_VAR cx11_override.cc _tmp_src
)
unset(_tmp_src)

if(CXX11_OVERRIDE)
  config_h_add_compile_definitions(CXX11_OVERRIDE=override)
  message(STATUS "Checking whether the C++ compiler supports `override` keyword - yes")
else()
  config_h_add_compile_definitions(CXX11_OVERRIDE=)
  message(STATUS "Checking whether the C++ compiler supports `override` keyword - no")
endif()

if(ARIA2_STATIC)
  if(MINGW)
    # Define _USE_32BIT_TIME_T because 32bit library of MinGW-w64
    # does not implement many 64bit version functions.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_32BIT_TIME_T")
  endif()
endif()

option(ENABLE_GNUTIL_SYSTEM_CRYPTO_POLICY "Enable gnutls system wide crypto policy" off)

if(ENABLE_GNUTIL_SYSTEM_CRYPTO_POLICY)
  set(USE_GNUTLS_SYSTEM_CRYPTO_POLICY yes CACHE BOOL "Define to 1 if using gnutls system wide crypto policy .")
endif()

# #### Check for Libraries #####
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckIncludeFiles)
include(CheckIncludeFileCXX)
include(CheckTypeSize)
include(CheckFunctionExists)

if(WIN32)
  if(MSVC)
    set(a2_extra_windows_flags "/D WIN32_LEAN_AND_MEAN /FI windows.h")
  else()
    set(a2_extra_windows_flags "-DWIN32_LEAN_AND_MEAN -include windows.h")
  endif()

  set(CMAKE_REQUIRED_FLAGS ${a2_extra_windows_flags})
elseif(UNIX)
  set(CMAKE_REQUIRED_FLAGS "-D_GNU_SOURCE")
endif()

find_package(CPPUNIT)

if(CPPUNIT_FOUND)
  set(HAVE_CPPUNIT yes CACHE BOOL "Define if you have CPPUNIT" FORCE)
else()
  set(HAVE_CPPUNIT no CACHE BOOL "Define if you have CPPUNIT" FORCE)
endif()

find_package(ZLIB 1.2.3)

if(ZLIB_FOUND)
  set(HAVE_ZLIB yes CACHE BOOL "Define if you have zlib" FORCE)
  set(ZLIB_CFLAGS "")
  set(ZLIB_LIBS ${ZLIB_LIBRARIES})
  config_h_add_compile_definitions(HAVE_ZLIB=1)

  set(save_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
  set(save_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
  set(CMAKE_REQUIRED_LIBRARIES ${ZLIB_LIBRARIES})
  set(CMAKE_REQUIRED_INCLUDES ${ZLIB_INCLUDE_DIRS})

  check_cxx_symbol_exists(gzbuffer zlib.h HAVE_GZBUFFER)

  if(HAVE_GZBUFFER)
    config_h_add_compile_definitions(HAVE_GZBUFFER)
  endif()

  check_cxx_symbol_exists(gzsetparams zlib.h HAVE_GZSETPARAMS)

  if(HAVE_GZSETPARAMS)
    config_h_add_compile_definitions(HAVE_GZSETPARAMS)
  endif()

  set(CMAKE_REQUIRED_LIBRARIES ${save_CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_INCLUDES ${save_CMAKE_REQUIRED_INCLUDES})

  set(aria2_LIBS ${ZLIB_LIBRARIES} ${aria2_LIBS})
else()
  set(HAVE_ZLIB no CACHE BOOL "Define if you have zlib" FORCE)
endif()

find_package(LibUV 1.13)

if(LIBUV_FOUND)
  set(HAVE_LIBUV yes CACHE BOOL "Define if you have libuv" FORCE)
  set(LIBUV_CFLAGS "")
  set(LIBUV_LIBS ${LIBUV_LIBRARIES})
  config_h_add_compile_definitions(HAVE_LIBUV=1)
else()
  set(HAVE_LIBUV no CACHE BOOL "Define if you have libuv" FORCE)
endif()

find_package(LibXml2)

if(LibXml2_FOUND)
  set(HAVE_LIBXML2 yes CACHE BOOL "Define if you have libxml2" FORCE)
  set(LIBXML2_CFLAGS "")
  set(LIBXML2_LIBS ${LIBXML2_LIBRARIES})
  config_h_add_compile_definitions(HAVE_LIBXML2=1)
else()
  set(HAVE_LIBXML2 no CACHE BOOL "Define if you have libxml2" FORCE)
endif()

find_package(EXPAT)

if(EXPAT_FOUND AND NOT HAVE_LIBXML2)
  set(HAVE_LIBEXPAT yes CACHE BOOL "Define if you have libexpat" FORCE)
  set(LIBEXPAT_CFLAGS "")
  set(LIBEXPAT_LIBS ${EXPAT_LIBRARIES})
  config_h_add_compile_definitions(HAVE_LIBEXPAT=1)
else()
  set(HAVE_LIBEXPAT no CACHE BOOL "Define if you have libexpat" FORCE)
endif()

find_package(SQLite3)

if(SQLite3_FOUND)
  set(HAVE_SQLITE3 yes CACHE BOOL "Define if you have sqlite3" FORCE)
  set(SQLITE3_CFLAGS "")
  set(SQLITE3_LIBS ${SQLite3_LIBRARIES})
  config_h_add_compile_definitions(HAVE_SQLITE3=1)

  set(save_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
  set(save_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
  set(CMAKE_REQUIRED_LIBRARIES ${SQLite3_LIBRARIES})
  set(CMAKE_REQUIRED_INCLUDES ${SQLite3_INCLUDE_DIRS})

  check_cxx_symbol_exists(sqlite3_open_v2 sqlite3.h HAVE_SQLITE3_OPEN_V2)

  if(HAVE_SQLITE3_OPEN_V2)
    config_h_add_compile_definitions(HAVE_SQLITE3_OPEN_V2)
  endif()

  set(CMAKE_REQUIRED_LIBRARIES ${save_CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_INCLUDES ${save_CMAKE_REQUIRED_INCLUDES})
else()
  set(HAVE_SQLITE3 no CACHE BOOL "Define if you have sqlite3" FORCE)
endif()

if(APPLE)
  set(HAVE_OSX yes CACHE BOOL "On Mac OS X" FORCE)
else()
  set(HAVE_OSX no CACHE BOOL "On Mac OS X" FORCE)
endif()


set(a2_found_headers)

if(WIN32)
  set(WIN_BUILD yes)

  set(_check_headers
    windows.h
    winsock2.h
    ws2tcpip.h
    mmsystem.h
    io.h
    iphlpapi.h
    winioctl.h
    share.h
    sys/utime.h
  )

  foreach(header ${_check_headers})
    string(TOUPPER ${header} header_var)
    string(REPLACE "." "_" header_var ${header_var})
    string(REPLACE "/" "_" header_var ${header_var})
    set(header_var "HAVE_${header_var}")
    set(${header_var})
    check_include_file_cxx(${header} ${header_var})

    if(${header_var})
      list(APPEND a2_found_headers ${header})
      config_h_add_compile_definitions(${header_var})
    else()
    endif()
  endforeach()

  unset(_check_headers)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Cake can't find them but they absolutly exist
    set(HAVE_IPHLPAPI_H true)
    set(HAVE_WINIOCTL_H true)
    config_h_add_compile_definitions(HAVE_IPHLPAPI_H)
    config_h_add_compile_definitions(HAVE_WINIOCTL_H)
    
  endif()

else()
  set(WIN_BUILD no)
endif()

set(HAVE_APPLETLS no CACHE BOOL "Define if you have Apple TLS" FORCE)

if(WITH_APPLETLS)
  if(HAVE_OSX)
    set(HAVE_APPLETLS yes CACHE BOOL "Define if you have Apple TLS" FORCE)
    config_h_add_compile_definitions(HAVE_APPLETLS=1)
    set(APPLETLS_LD_FLAGS "-framework CoreFoundation" "-framework Security")
    set(HAVE_SSL yes CACHE BOOL "Have SSL support" FORCE)
    set(HAVE_NATIVETLS yes CACHE BOOL "Have Native TLS" FORCE)
    message(STATUS "Enable Mac OS X native SSL/TLS support")
  endif()
endif()

function(a2_wrap_find_library var_name lib)
  message(STATUS "Looking for ${lib}")
  find_library(${var_name} ${lib})
  if(${var_name})
    message(STATUS "Looking for ${lib} - found")
  else()
    message(STATUS "Looking for ${lib} - not found")
  endif()
endfunction()

set(HAVE_WINTLS no CACHE BOOL "Define if you have Windows TLS support" FORCE)
set(HAVE_WINTLS_LIBS false)
set(HAVE_WINTLS_HEADERS false)
if(WITH_WINTLS)
  if(WIN32)
    a2_wrap_find_library(HAVE_CRYPT32 crypt32)
    a2_wrap_find_library(HAVE_SECURE32 secur32)
    a2_wrap_find_library(HAVE_ADVAPI32 advapi32)

    if(HAVE_CRYPT32 AND HAVE_SECURE32 AND HAVE_ADVAPI32)
      set(HAVE_WINTLS_LIBS true)
    endif()

    check_include_file_cxx(wincrypt.h HAVE_WINCRYPT_H)
    if(HAVE_WINCRYPT_H)
      list(APPEND a2_found_headers wincrypt.h)
    endif()

    if(MSVC)
      list(APPEND CMAKE_REQUIRED_DEFINITIONS "/D SECURITY_WIN32=1")
    else()
      list(APPEND CMAKE_REQUIRED_DEFINITIONS -DSECURITY_WIN32=1)
    endif()
    check_include_file_cxx(security.h HAVE_SECURITY_H)
    if(HAVE_SECURITY_H)
      list(APPEND a2_found_headers security.h)
    else()
      list(POP_BACK CMAKE_REQUIRED_DEFINITIONS)
    endif()
    
    if(HAVE_WINCRYPT_H AND HAVE_SECURITY_H)
      set(HAVE_WINTLS_HEADERS true)
    endif()

    if(HAVE_WINTLS_LIBS AND HAVE_WINTLS_HEADERS)
      set(HAVE_WINTLS yes CACHE BOOL "Define if you have Windows TLS support" FORCE)
      config_h_add_compile_definitions(HAVE_WINTLS=1)
      config_h_add_compile_definitions(SECURITY_WIN32=1) # Use security.h in WIN32 mode
      set(WINTLS_LIBS crypt32 secur32 advapi32)
      set(HAVE_SSL yes CACHE BOOL "Have SSL support" FORCE)
      set(HAVE_NATIVETLS yes CACHE BOOL "Have Native TLS" FORCE)
      message(STATUS "Enable Windows native SSL/TLS support")
    endif()
  endif()
endif()

set(HAVE_LIBGNUTLS no CACHE BOOL "Define if you have libguntls" FORCE)

if(WITH_GNUTLS)
  find_package(GnuTLS 2.2.0)

  if(GNUTLS_FOUND AND NOT HAVE_SSL)
    set(HAVE_LIBGNUTLS yes CACHE BOOL "Define if you have libguntls" FORCE)
    set(LIBGNUTLS_CFLAGS "${GNUTLS_DEFINITIONS}")
    set(LIBGNUTLS_LIBS ${GNUTLS_LIBRARIES})
    config_h_add_compile_definitions(HAVE_LIBGNUTLS=1)
    set(HAVE_SSL yes CACHE BOOL "Have SSL support" FORCE)

    set(save_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    set(save_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
    set(save_CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS})
    set(CMAKE_REQUIRED_LIBRARIES ${GNUTLS_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${GNUTLS_INCLUDE_DIR})
    set(CMAKE_REQUIRED_DEFINITIONS ${GNUTLS_DEFINITIONS})

    check_cxx_symbol_exists(gnutls_certificate_set_x509_system_trust gnutls/gnutls.h HAVE_GNUTLS_CERTIFICATE_SET_X509_SYSTEM_TRUST)

    if(HAVE_GNUTLS_CERTIFICATE_SET_X509_SYSTEM_TRUST)
      config_h_add_compile_definitions(HAVE_GNUTLS_CERTIFICATE_SET_X509_SYSTEM_TRUST)
    endif()

    set(CMAKE_REQUIRED_LIBRARIES ${save_CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${save_CMAKE_REQUIRED_INCLUDES})
    set(CMAKE_REQUIRED_DEFINITIONS ${save_CMAKE_REQUIRED_DEFINITIONS})
  endif()
endif()

set(HAVE_OPENSSL no CACHE BOOL "Define is you have openssl" FORCE)

if(WITH_OPENSSL)
  find_package(OpenSSL)

  if(OPENSSL_FOUND AND NOT HAVE_SSL)
    set(HAVE_OPENSSL yes CACHE BOOL "Define is you have openssl" FORCE)
    set(OPENSSL_CFLAGS "")
    set(OPENSSL_LIBS ${OPENSSL_LIBRARIES})
    config_h_add_compile_definitions(HAVE_OPENSSL=1)
    set(HAVE_SSL yes CACHE BOOL "Have SSL support" FORCE)

    set(save_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    set(save_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
    set(CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})

    check_cxx_symbol_exists(EVP_DigestInit_ex openssl/evp.h HAVE_EVP_DIGESTINIT_EX)

    if(HAVE_EVP_DIGESTINIT_EX)
      set(HAVE_OLD_OPENSSL yes CACHE BOOL "Define if you have old openssl" FORCE)
      config_h_add_compile_definitions(HAVE_OLD_OPENSSL=1)
    endif()

    check_cxx_symbol_exists(EVP_sha224 openssl/evp.h HAVE_EVP_SHA224)

    if(HAVE_EVP_SHA224)
      config_h_add_compile_definitions(HAVE_EVP_SHA224)
    endif()

    check_cxx_symbol_exists(EVP_sha256 openssl/evp.h HAVE_EVP_SHA256)

    if(HAVE_EVP_SHA256)
      config_h_add_compile_definitions(HAVE_EVP_SHA256)
    endif()

    check_cxx_symbol_exists(EVP_sha384 openssl/evp.h HAVE_EVP_SHA384)

    if(HAVE_EVP_SHA384)
      config_h_add_compile_definitions(HAVE_EVP_SHA384)
    endif()

    check_cxx_symbol_exists(EVP_sha512 openssl/evp.h HAVE_EVP_SHA512)

    if(HAVE_EVP_SHA512)
      config_h_add_compile_definitions(HAVE_EVP_SHA512)
    endif()

    set(CMAKE_REQUIRED_LIBRARIES ${save_CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${save_CMAKE_REQUIRED_INCLUDES})
  endif()
endif()

set(HAVE_LIBNETTLE no CACHE BOOL "Define if you have libnettle" FORCE)
set(HAVE_LIBGMP no CACHE BOOL "Define if you have GNU Multiple Precision Arithmetis Library (GMP)" FORCE)
set(HAVE_LIBGCRYPT no CACHE BOOL "Define is you have LibGcrypt" FORCE)

if(NOT HAVE_OPENSSL)
  if(WITH_LIBNETTLE)
    find_package(Nettle)

    if(NETTLE_FOUND AND NOT HAVE_NATIVETLS)
      set(HAVE_LIBNETTLE yes CACHE BOOL "Define if you have libnettle" FORCE)
      set(LIBNETTLE_CFLAGS "")
      set(LIBNETTLE_LIBS ${NETTLE_LIBRARIES})
      config_h_add_compile_definitions(HAVE_LIBNETTLE=1)
    endif()
  endif()

  if(WITH_LIBGMP)
    find_package(GMP)

    if(GMP_FOUND AND(HAVE_LIBNETTLE OR HAVE_NATIVETLS) AND ENABLE_BITTORRENT)
      set(LIBGMP_LIBS ${GMP_LIBRARIES})
      set(LIBGMP_CFLAGS "")
      set(HAVE_LIBGMP yes CACHE BOOL "Define if you have GNU Multiple Precision Arithmetis Library (GMP)" FORCE)
      config_h_add_compile_definitions(HAVE_LIBGMP=1)
      set(save_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
      set(save_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
      set(CMAKE_REQUIRED_LIBRARIES ${GMP_LIBRARIES})
      set(CMAKE_REQUIRED_INCLUDES ${GMP_INCLUDE_DIR})
      check_cxx_symbol_exists(__gmpz_powm_sec gmp.h HAVE_MPZ_POWM_SEC)

      if(HAVE_MPZ_POWM_SEC)
        config_h_add_compile_definitions(HAVE_GMP_SEC=1)
      endif()

      set(CMAKE_REQUIRED_LIBRARIES ${save_CMAKE_REQUIRED_LIBRARIES})
      set(CMAKE_REQUIRED_INCLUDES ${save_CMAKE_REQUIRED_INCLUDES})
    endif()
  endif()

  if(WITH_LIBGCRYPT)
    find_package(LibGcrypt)

    if(LibGcrypt_FOUND AND NOT HAVE_NATIVETLS AND NOT HAVE_LIBNETTLE)
      set(LIBGCRYPT_CFLAGS "")
      set(LIBGCRYPT_LIBS ${LibGcrypt_LIBRARIES})
      set(HAVE_LIBGCRYPT yes CACHE BOOL "Define is you have LibGcrypt" FORCE)
      config_h_add_compile_definitions(HAVE_LIBGCRYPT=1)
    endif()
  endif()
endif()

set(HAVE_LIBSSH2 no CACHE BOOL "Define if you have LibSSH2" FORCE)

if(WITH_LIBSSH2)
  find_package(LibSSH2)

  if(LIBSSH2_FOUND)
    set(HAVE_LIBSSH2 yes CACHE BOOL "Define if you have LibSSH2" FORCE)
    set(LIBSSH2_CFLAGS "")
    set(LIBSSH2_LIBS ${LIBSSH2_LIBRARIES})
    config_h_add_compile_definitions(HAVE_LIBSSH2=1)
  endif()
endif()

set(HAVE_LIBCARES no CACHE BOOL "Define if you hae libcares" FORCE)

if(WITH_LIBCARES)
  find_package(Libcares)

  if(LIBCARES_FOUND)
    set(HAVE_LIBCARES yes CACHE BOOL "Define if you hae libcares" FORCE)
    set(LIBCARES_CFLAGS "")
    set(LIBCARES_LIBS ${LIBCARES_LIBRARIES})
    config_h_add_compile_definitions(HAVE_LIBCARES=1)

    set(save_CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES})
    set(save_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    set(save_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})

    set(CMAKE_REQUIRED_LIBRARIES ${LIBCARES_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${LIBCARES_INCLUDE_DIR})
    set(CMAKE_EXTRA_INCLUDE_FILES ares.h)
    check_type_size(ares_addr_node ARES_ADDR_NODE LANGUAGE CXX)
   

    if(ARES_ADDR_NODE)
      config_h_add_compile_definitions(HAVE_ARES_ADDR_NODE)
    endif()

    check_cxx_symbol_exists(ares_set_servers ares.h HAVE_ARES_SET_SERVERS)

    if(HAVE_ARES_SET_SERVERS)
      config_h_add_compile_definitions(HAVE_ARES_SET_SERVERS)
    endif()

    set(CMAKE_REQUIRED_LIBRARIES ${save_CMAKE_REQUIRED_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${save_CMAKE_REQUIRED_INCLUDES})
    set(CMAKE_EXTRA_INCLUDE_FILES ${save_CMAKE_EXTRA_INCLUDE_FILES})
  endif()
endif()

set(USE_MD "" CACHE STRING "What message digest implementation to use")

if(HAVE_APPLETLS)
  set(USE_MD "apple" CACHE STRING "What message digest implementation to use" FORCE)
  set(USE_APPLE_MD true CACHE BOOL "use Apple TLS message digest" FORCE)
  config_h_add_compile_definitions(USE_APPLE_MD=1)
elseif(HAVE_LIBNETTLE)
  set(USE_MD "libnettle" CACHE STRING "What message digest implementation to use" FORCE)
  set(USE_LIBNETTLE_MD true CACHE BOOL "use Nettle message digest" FORCE)
  config_h_add_compile_definitions(USE_LIBNETTLE_MD=1)
elseif(HAVE_LIBGCRYPT)
  set(USE_MD "libgcrypt" CACHE STRING "What message digest implementation to use" FORCE)
  set(USE_LIBGCRYPT_MD true CACHE BOOL "use Gcrypt message digest" FORCE)
  config_h_add_compile_definitions(USE_LIBGCRYPT_MD=1)
elseif(HAVE_OPENSSL)
  set(USE_MD "openssl" CACHE STRING "What message digest implementation to use" FORCE)
  set(USE_OPENSSL_MD true CACHE BOOL "use OpenSSL message digest" FORCE)
  config_h_add_compile_definitions(USE_OPENSSL_MD=1)
else()
  set(USE_MD "internal" CACHE STRING "What message digest implementation to use" FORCE)
  set(USE_INTERNAL_MD true CACHE BOOL "use internal message digest impl" FORCE)
  config_h_add_compile_definitions(USE_INTERNAL_MD=1)
endif()

if(HAVE_SSL)
  set(ENABLE_SSL yes CACHE BOOL "If SSL support is enabled" FORCE)
else()
  set(ENABLE_SSL no CACHE BOOL "If SSL support is enabled" FORCE)
endif()

if(ENABLE_SSL)
  config_h_add_compile_definitions(ENABLE_SSL=1)
endif()

if(HAVE_LIBGMP OR HAVE_LIBGCRYPT OR HAVE_OPENSSL)
  set(USE_INTERNAL_BIGNUM no)
else()
  set(USE_INTERNAL_BIGNUM yes)
endif()

if(USE_INTERNAL_BIGNUM)
  config_h_add_compile_definitions(USE_INTERNAL_BIGNUM=1)
endif()

if(HAVE_LIBNETTLE OR HAVE_LIBGCRYPT OR HAVE_OPENSSL)
  set(USE_INTERNAL_ARC4 no)
else()
  set(USE_INTERNAL_ARC4 yes)
endif()

if(USE_INTERNAL_ARC4)
  config_h_add_compile_definitions(USE_INTERNAL_ARC4)
endif()

if(ENABLE_BITTORRENT)
  config_h_add_compile_definitions(ENABLE_BITTORRENT=1)
endif()

if((HAVE_LIBXML2 OR HAVE_LIBEXPAT) AND ENABLE_METALINK)
  config_h_add_compile_definitions(ENABLE_METALINK=1)
endif()

if(HAVE_LIBXML2 OR HAVE_LIBEXPAT)
  set(HAVE_SOME_XMLLIB yes CACHE BOOL "either libxml2 or libxpax present" FORCE)
  config_h_add_compile_definitions(HAVE_SOME_XMLLIB=1)
  set(ENABLE_XML_RPC yes CACHE BOOL "XML RPC Enabled")
else()
  set(HAVE_SOME_XMLLIB no CACHE BOOL "either libxml2 or libxpax present" FORCE)
  set(ENABLE_XML_RPC no CACHE BOOL "XML RPC Enabled" FORCE)
endif()

if(ENABLE_XML_RPC)
  config_h_add_compile_definitions(ENABLE_XML_RPC=1)
endif()

if(HAVE_LIBCARES)
  set(ENABLE_ASYNC_DNS yes CACHE BOOL "Async DNS Enabled")
else()
  set(ENABLE_ASYNC_DNS no CACHE BOOL "Async DNS Enabled" FORCE)
endif()

if(ENABLE_ASYNC_DNS)
  config_h_add_compile_definitions(ENABLE_ASYNC_DNS=1)
endif()

check_cxx_symbol_exists(alloca "alloca.h;stdlib.h;cstdlib;malloc.h" HAVE_ALLOCA_H)

if(HAVE_ALLOCA_H)
  config_h_add_compile_definitions(HAVE_ALLOCA_H)
endif()

set(_check_headers
  argz.h
  arpa/inet.h
  fcntl.h
  float.h
  inttypes.h
  langinfo.h
  libintl.h
  limits.h
  libgen.h
  locale.h
  malloc.h
  math.h
  memory.h
  netdb.h
  netinet/in.h
  netinet/tcp.h
  poll.h
  port.h
  signal.h
  stddef.h
  stdint.h
  stdio_ext.h
  stdlib.h 
  string.h
  strings.h
  sys/epoll.h
  sys/event.h
  sys/ioctl.h
  sys/mman.h
  sys/param.h
  sys/resource.h
  sys/stat.h
  sys/signal.h
  sys/socket.h
  sys/time.h
  sys/types.h
  sys/uio.h
  sys/utsname.h
  termios.h
  time.h
  unistd.h
  utime.h
  wchar.h
  ifaddrs.h
  pwd.h 
)

foreach(header ${_check_headers})
  string(TOUPPER ${header} header_var)
  string(REPLACE "." "_" header_var ${header_var})
  string(REPLACE "/" "_" header_var ${header_var})
  set(header_var "HAVE_${header_var}")
  set(${header_var})
  
  check_include_file_cxx(${header} ${header_var})

  if(${header_var})
    list(APPEND a2_found_headers ${header})
    config_h_add_compile_definitions(${header_var})
  else()
  endif()
endforeach()

unset(_check_headers)

check_include_file_cxx(stdbool.h HAVE_STDBOOL_H)

if(HAVE_STDBOOL_H)
  list(APPEND a2_found_headers stdbool.h)
  config_h_add_compile_definitions(HAVE_STDBOOL_H=1)
endif()

if(cxx_constexpr IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  message(STATUS "ANSI C-confirming const - yes")
else()
  message(STATUS "ANSI C-confirming const - no")
endif()

set(save_CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES})
set(CMAKE_EXTRA_INCLUDE_FILES ${a2_found_headers})
check_type_size("struct timespec" HAVE_A2_STRUCT_TIMESPEC LANGUAGE CXX)
set(CMAKE_EXTRA_INCLUDE_FILES ${save_CMAKE_EXTRA_INCLUDE_FILES})

if(HAVE_A2_STRUCT_TIMESPEC)
  set(HAVE_A2_STRUCT_TIMESPEC true)
  config_h_add_compile_definitions(HAVE_A2_STRUCT_TIMESPEC)
endif()

find_package(Gettext)

set(save_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
if(WIN32)
  # No exta libs needed
else()
  list(APPEND CMAKE_REQUIRED_LIBRARIES m)
endif()
set(_check_funcs
  __argz_count
  __argz_next
  __argz_stringify
  atexit
  ftruncate
  getcwd
  getentropy
  gethostbyaddr
  gethostbyname
  getifaddrs
  getpagesize
  memchr
  memmove
  mempcpy
  memcpy
  memset
  mkdir
  mmap
  munmap
  nl_langinfo
  posix_fadvise
  posix_memalign
  pow
  putenv
  rmdir
  select
  setlocale
  sigaction
  sleep
  socket
  stpcpy
  strcasecmp
  strchr
  strcspn
  strdup
  strerror
  strncasecmp
  strstr
  strtol
  strtoul
  strtoull
  tzset
  unsetenv
  usleep
  utime
  utimes
)

foreach(func ${_check_funcs})
  string(TOUPPER ${func} func_var)
  set(func_var "HAVE_${func_var}")
  set(${func_var})

  # check_cxx_symbol_exists(${func} "${a2_found_headers}" ${func_var})
  check_symbol_exists(${func} "${a2_found_headers}" ${func_var})

  if(${func_var})
    config_h_add_compile_definitions(${func_var})
  else()
  endif()
endforeach()

unset(_check_funcs)
set(CMAKE_REQUIRED_LIBRARIES ${save_CMAKE_REQUIRED_LIBRARIES})

if(WITH_TCMALLOC AND WITH_JEMALLOC)
  message(FATAL_ERROR "Cannot use both, tcmalloc and jemalloc!")
endif()

set(HAVE_TCMALLOC no CACHE BOOL "If you have tcmalloc" FORCE)

if(WITH_TCMALLOC)
  find_package(Tcmalloc)

  if(Tcmalloc_FOUND)
    set(HAVE_TCMALLOC yes CACHE BOOL "If you have tcmalloc" FORCE)
  endif()
endif()

set(HAVE_JEMALLOC no CACHE BOOL "if you have jemalloc" FORCE)

if(WITH_JEMALLOC)
  find_package(Jemalloc)

  if(Jemalloc_FOUND)
    set(HAVE_JEMALLOC yes CACHE BOOL "if you have jemalloc" FORCE)
  endif()
endif()

if(ENABLE_EPOLL)
  check_cxx_symbol_exists(epoll_create "${a2_found_headers}" HAVE_EPOLL)

  if(HAVE_EPOLL)
    config_h_add_compile_definitions(HAVE_EPOLL=1)
  endif()
endif()

check_cxx_symbol_exists(posix_fallocate "${a2_found_headers}" HAVE_POSIX_FALLOCATE)
if(NOT HAVE_POSIX_FALLOCATE)
  message(STATUS "Retrying with check_function_exists ...")
  unset(HAVE_POSIX_FALLOCATE CACHE)
  check_function_exists(posix_fallocate HAVE_POSIX_FALLOCATE)
endif()
if(HAVE_POSIX_FALLOCATE)
  config_h_add_compile_definitions(HAVE_POSIX_FALLOCATE=1)
endif()

check_cxx_symbol_exists(fallocate "${a2_found_headers}" HAVE_FALLOCATE)
if(NOT HAVE_FALLOCATE)
  message(STATUS "Retrying with check_function_exists ...")
  unset(HAVE_FALLOCATE CACHE)
  check_function_exists(fallocate HAVE_FALLOCATE)
endif()
if(HAVE_FALLOCATE)
  config_h_add_compile_definitions(HAVE_FALLOCATE=1)
endif()

if(HAVE_POSIX_FALLOCATE OR HAVE_FALLOCATE OR HAVE_OSX OR WIN_BUILD)
  set(HAVE_SOME_FALLOCATE yes CACHE BOOL "if there is some fallocate support" FORCE)
  config_h_add_compile_definitions(HAVE_SOME_FALLOCATE=1)
else()
  set(HAVE_SOME_FALLOCATE no CACHE BOOL "if there is some fallocate support" FORCE)
endif()

check_cxx_symbol_exists(asctime_r "${a2_found_headers}" HAVE_ASCTIME_R)

if(HAVE_ASCTIME_R)
  config_h_add_compile_definitions(HAVE_ASCTIME_R=1)
endif()

check_cxx_symbol_exists(localtime_r "${a2_found_headers}" HAVE_LOCALTIME_R)

if(HAVE_LOCALTIME_R)
  config_h_add_compile_definitions(HAVE_LOCALTIME_R=1)
endif()

set(_check_funcs
  basename
  gai_strerror
  getaddrinfo
  gettimeofday
  strptime
  daemon
  poll
  port_associate
  kqueue
)

foreach(func ${_check_funcs})
  string(TOUPPER ${func} func_var)
  set(func_var "HAVE_${func_var}")
  set(${func_var})

  check_symbol_exists(${func} "${a2_found_headers}" ${func_var})

  if(${func_var})
    config_h_add_compile_definitions(${func_var})
  else()
  endif()
endforeach()

unset(_check_funcs)

if(NOT HAVE_GAI_STRERR AND HAVE_WS2TCPIP_H AND (WIN32 AND NOT MINGW))
  set(HAVE_GAI_STRERROR true) # It's there in ws2tcpip.h
  config_h_add_compile_definitions(HAVE_GAI_STRERROR)
  set(HAVE_GETADDRINFO true) # It's there in ws2tcpip.h
  config_h_add_compile_definitions(HAVE_GETADDRINFO)
endif()

if(HAVE_KQUEUE)
  message(STATUS "Checking whether struct kevent.udata is intptr_t")
  set(_tmp_src
    "\
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
int main(int argc, char* argv[]) {
    struct kevent event;
    event.udata = reinterpret_cast<intptr_t>(&event);
}
"
  )
  try_compile(KEVENT_UDATA_INTPTR_T
    SOURCE_FROM_VAR kevent_udata_intptr_t_src.c _tmp_src
  )
  unset(_tmp_src)

  if(KEVENT_UDATA_INTPTR_T)
    message(STATUS "Checking whether struct kevent.udata is intptr_t - yes")
    config_h_add_compile_definitions(KEVENT_UDATA_INTPTR_T=1)
  else()
    message(STATUS "Checking whether struct kevent.udata is intptr_t - no")
  endif()
endif()

set(save_CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES})
set(CMAKE_EXTRA_INCLUDE_FILES  ${a2_found_headers})
check_type_size("((struct sockaddr_in*)0)->sin_len" HAVE_SOCKADDR_IN_SIN_LEN)

if(HAVE_SOCKADDR_IN_SIN_LEN)
  config_h_add_compile_definitions(HAVE_SOCKADDR_IN_SIN_LEN=1)
endif()

check_type_size("((struct sockaddr_in6*)0)->sin6_len" HAVE_SOCKADDR_IN6_SIN6_LEN)

if(HAVE_SOCKADDR_IN6_SIN6_LEN)
  config_h_add_compile_definitions(HAVE_SOCKADDR_IN6_SIN6_LEN=1)
endif()

set(CMAKE_EXTRA_INCLUDE_FILES ${save_CMAKE_EXTRA_INCLUDE_FILES})

message(STATUS "Checking whether struct option.name is assignable from const char*")
set(_tmp_src
  "\
#include <unistd.h>
#include <getopt.h>
int main(int argc, char* argv[]) {
    const char* s = \"const char\";
    struct option op;
    op.name = s; 
}
"
)
try_compile(HAVE_OPTION_CONST_NAME
  SOURCE_FROM_VAR option_const_name.c _tmp_src
)
unset(_tmp_src)

if(HAVE_OPTION_CONST_NAME)
  message(STATUS "Checking whether struct option.name is assignable from const char* - yes")
  config_h_add_compile_definitions(HAVE_OPTION_CONST_NAME=1)
else()
  message(STATUS "Checking whether struct option.name is assignable from const char* - no")
endif()

if(ENABLE_WEBSOCKET)
  config_h_add_compile_definitions(ENABLE_WEBSOCKET=1)
endif()

if(ARIA2_STATIC)
  if(NOT WIN32 AND NOT APPLE)
    set(EXTRALIBS -lpthread -ldl -lrt ${EXTRALIBS})
  endif()
endif()

if(ENABLE_WERROR)
  add_compile_options(-Wall -Werror -Wformat-security)
endif()

add_link_options(${EXTRALIBS})

config_h_generate_header(config)
# pickup the generated config.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compile_definitions(HAVE_CONFIG_H)

# add_subdirectory(po)
add_subdirectory(lib)
add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(test)

message(STATUS "summary of build options: \n\

Build:          ${BUILD_ID}
Host:           ${HOST_ID}
Target:         ${TARGET_ID}
Install prefix: ${CMAKE_INSTALL_PREFIX}
CC:             ${CMAKE_C_COMPILER}
CXX:            ${CMAKE_CXX_COMPILER}
CPP:            ${CMAKE_CPP_COMPILER}
CXXFLAGS:       ${CMAKE_CXX_FLAGS}
CFLAGS:         ${CMAKE_C_FLAGS}
CPPFLAGS:       ${CMAKE_CPP_FLAGS}
LDFLAGS:        EXE: ${CMAKE_EXE_LINKER_FLAGS} SHARED: ${CMAKE_SHARED_LINKER_FLAGS}
LIBS:           ${LIBS}
DEFS:           ${DEFS}
CXX1XCXXFLAGS:  ${CXX1XCXXFLAGS}
EXTRACXXFLAGS:  ${EXTRACXXFLAGS}
EXTRACFLAGS:    ${EXTRACFLAGS}
EXTRACPPFLAGS:  ${EXTRACPPFLAGS}
EXTRALDFLAGS:   ${EXTRALDFLAGS}
EXTRALIBS:      ${EXTRALIBS}
WARNCXXFLAGS:   ${WARNCXXFLAGS}
LibUV:          ${HAVE_LIBUV} (CFLAGS='${LIBUV_CFLAGS}' LIBS='${LIBUV_LIBS}')
SQLite3:        ${HAVE_SQLITE3} (CFLAGS='${SQLITE3_CFLAGS}' LIBS='${SQLITE3_LIBS}')
SSL Support:    ${HAVE_SSL}
AppleTLS:       ${HAVE_APPLETLS} (LDFLAGS='${APPLETLS_LDFLAGS}')
WinTLS:         ${HAVE_WINTLS} (LIBS='${WINTLS_LIBS}')
GnuTLS:         ${HAVE_LIBGNUTLS} (CFLAGS='${LIBGNUTLS_CFLAGS}' LIBS='${LIBGNUTLS_LIBS}')
OpenSSL:        ${HAVE_OPENSSL} (CFLAGS='${OPENSSL_CFLAGS}' LIBS='${OPENSSL_LIBS}')
CA Bundle:      ${CA_BUNDLE}
LibNettle:      ${HAVE_LIBNETTLE} (CFLAGS='${LIBNETTLE_CFLAGS}' LIBS='${LIBNETTLE_LIBS}')
LibGmp:         ${HAVE_LIBGMP} (CFLAGS='${LIBGMP_CFLAGS}' LIBS='${LIBGMP_LIBS}')
LibGcrypt:      ${HAVE_LIBGCRYPT} (CFLAGS='${LIBGCRYPT_CFLAGS}' LIBS='${LIBGCRYPT_LIBS}')
LibXML2:        ${HAVE_LIBXML2} (CFLAGS='${LIBXML2_CFLAGS}' LIBS='${LIBXML2_LIBS}')
LibExpat:       ${HAVE_LIBEXPAT} (CFLAGS='${EXPAT_CFLAGS}' LIBS='${EXPAT_LIBS}')
LibCares:       ${HAVE_LIBCARES} (CFLAGS='${LIBCARES_CFLAGS}' LIBS='${LIBCARES_LIBS}')
Zlib:           ${HAVE_ZLIB} (CFLAGS='${ZLIB_CFLAGS}' LIBS='${ZLIB_LIBS}')
Libssh2:        ${HAVE_LIBSSH2} (CFLAGS='${LIBSSH2_CFLAGS}' LIBS='${LIBSSH2_LIBS}')
Tcmalloc:       ${HAVE_TCMALLOC} (CFLAGS='${TCMALLOC_CFLAGS}' LIBS='${TCMALLOC_LIBS}')
Jemalloc:       ${HAVE_JEMALLOC} (CFLAGS='${JEMALLOC_CFLAGS}' LIBS='${JEMALLOC_LIBS}')
Epoll:          ${HAVE_EPOLL}
Bittorrent:     ${ENABLE_BITTORRENT}
Metalink:       ${ENABLE_METALINK}
XML-RPC:        ${ENABLE_XML_RPC}
Message Digest: ${USE_MD}
WebSocket:      ${ENABLE_WEBSOCKET} (CFLAGS='${WSLAY_CFLAGS}' LIBS='${WSLAY_LIBS}')
Libaria2:       ${ENABLE_LIBARIA2} (shared=${enable_shared} static=${enable_static})
bash_completion dir: ${BASHCOMPLETIONDIR}
Static build:   ${ARIA2_STATIC}
")

# #### README Generation #####
find_package(Docutils)

if(Docutils_FOUND)
  if(WIN32)
    find_package(PythonInterp)
  endif()

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/README.html
    COMMAND ${PYTHON_EXECUTABLE} ${RST2HTML_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/README.rst ${CMAKE_CURRENT_SOURCE_DIR}/README.html
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/README.rst
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/README.rst
  )
  add_custom_target(readme_html ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/README.html)
else()
  message(WARNING "rst2html not installed. Placing only a dummy file for README.html")
  write_file(${CMAKE_CURRENT_SOURCE_DIR}/README.html
    "<!doctype html>\n"
    "<p>Readme has not been built!.<br>Instead see <a href='README.rst'>README.rst</a></p>"
  )
endif()

file(GLOB ALL_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/includes/aria2/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h
)

add_custom_target(clang-format
  COMMAND clang-format -i ${ALL_SOURCE_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# ########## install files ###############
install(FILES README README.rst README.html TYPE DOC)
install(FILES LICENSE.OpenSSL DESTINATION TYPE DOC)

# original Makefile.am contents follow:

# SUBDIRS =  po lib deps src doc test
#
# ACLOCAL_AMFLAGS = -I m4 --install
# RST2HTML = @RST2HTML@
#
# EXTRA_DIST = config.rpath \
# script-helper \
# makerelease \
# mingw-config mingw-release mingw-build-memo \
# android-config \
# makerelease-osx.mk osx-package/DS_Store osx-package/etc/paths.d/aria2c osx-package/etc/manpaths.d/aria2 \
# Dockerfile.mingw \
# Dockerfile.raspberrypi \
# examples/libaria2ex.cc examples/libaria2wx.cc
#
# dist_doc_DATA = README README.rst README.html
#
# .PHONY: clang-format
#
# if HAVE_RST2HTML
# README.html: README.rst
# $(RST2HTML)  $< > $@
# else # !HAVE_RST2HTML
# README.html: README.rst
# @echo Warning: rst2html not installed. Placing only a dummy file for $@
# echo "<!doctype html>" > $@
# echo "<p>Readme has not been built!.<br>Instead see <a href='README.rst'>README.rst</a></p>" >> $@
# endif # !HAVE_RST2HTML
#
# dist_noinst_DATA = LICENSE.OpenSSL
#
# # Format source files using clang-format.  Don't format source files
# # under deps directory since we are not responsible for their coding
# # style.
# clang-format:
# CLANGFORMAT=`git config --get clangformat.binary`; \
# test -z $${CLANGFORMAT} && CLANGFORMAT="clang-format"; \
# $${CLANGFORMAT} -i $(top_srcdir)/src/*.{c,cc,h} $(top_srcdir)/src/includes/aria2/*.h \
# $(top_srcdir)/examples/*.cc $(top_srcdir)/test/*.{cc,h}
