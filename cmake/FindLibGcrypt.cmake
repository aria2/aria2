# Copyright 2014 Nicolás Alvarez <nicolas.alvarez@gmail.com>
# Copyright 2016, 2021 Igalia S.L
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#[=======================================================================[.rst:
FindGcrypt
----------

Find libgcrypt headers and libraries.

Imported Targets
^^^^^^^^^^^^^^^^

``LibGcrypt::LibGcrypt``
  The libgcrypt library, if found.

#]=======================================================================]

find_package(PkgConfig QUIET)
find_program(LIBGCRYPTCONFIG_SCRIPT NAMES libgcrypt-config)
if (PkgConfig_FOUND)
    # XXX: The libgcrypt.pc file does not list gpg-error as a dependency,
    #      resulting in linking errors; search for the latter as well.
    pkg_check_modules(PC_GCRYPT QUIET libgcrypt)
    pkg_check_modules(PC_GPGERROR QUIET gpg-error)
    set(LibGcrypt_COMPILE_OPTIONS ${PC_GCRYPT_CFLAGS_OTHER} ${PC_GPGERROR_CFLAGS_OTHER})
    set(LibGcrypt_VERSION ${PC_GCRYPT_VERSION})
endif ()

if (LIBGCRYPTCONFIG_SCRIPT AND NOT PC_GCRYPT)
    execute_process(
        COMMAND "${LIBGCRYPTCONFIG_SCRIPT}" --prefix
        RESULT_VARIABLE CONFIGSCRIPT_RESULT
        OUTPUT_VARIABLE LIBGCRYPT_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (CONFIGSCRIPT_RESULT EQUAL 0)
        set(LIBGCRYPT_SCRIPT_LIB_HINT "${LIBGCRYPT_PREFIX}/lib")
        set(LIBGCRYPT_SCRIPT_INCLUDE_HINT "${LIBGCRYPT_PREFIX}/include")
    endif ()

    execute_process(
        COMMAND "${LIBGCRYPTCONFIG_SCRIPT}" --cflags
        RESULT_VARIABLE CONFIGSCRIPT_RESULT
        OUTPUT_VARIABLE CONFIGSCRIPT_VALUE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (CONFIGSCRIPT_RESULT EQUAL 0)
        set(LibGcrypt_COMPILE_OPTIONS ${CONFIGSCRIPT_VALUE})
    endif ()

    execute_process(
        COMMAND "${LIBGCRYPTCONFIG_SCRIPT}" --version
        RESULT_VARIABLE CONFIGSCRIPT_RESULT
        OUTPUT_VARIABLE CONFIGSCRIPT_VALUE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (CONFIGSCRIPT_RESULT EQUAL 0)
        string(REGEX MATCH "^([0-9]+\.[0-9]+\.[0-9]+)" LibGcrypt_VERSION "${CONFIGSCRIPT_VALUE}")
    endif ()
endif ()

find_path(LibGcrypt_GpgError_INCLUDE_DIR
    NAMES gpg-error.h
    HINTS ${PC_GPGERROR_INCLUDEDIR} ${PC_GPGERROR_INCLUDE_DIRS}
          ${PC_GCRYPT_INCLUDEDIR} ${PC_GCRYPT_INCLUDE_DIRS}
          ${LIBGCRYPT_SCRIPT_INCLUDE_HINT} ${LibGcrypt_INCLUDE_DIR}
)

find_library(LibGcrypt_GpgError_LIBRARY
    NAMES ${LibGcrypt_GpgError_NAMES} gpg-error libgpg-error
    HINTS ${PC_GPGERROR_LIBDIR} ${PC_GPGERROR_LIBRARY_DIRS}
          ${PC_GCRYPT_LIBDIR} ${PC_GCRYPT_LIBRARY_DIRS} ${LIBGCRYPT_SCRIPT_LIB_HINT}
)

find_path(LibGcrypt_INCLUDE_DIR
    NAMES gcrypt.h
    HINTS ${PC_GCRYPT_INCLUDEDIR} ${PC_GCRYPT_INCLUDE_DIRS}
          ${LIBGCRYPT_SCRIPT_INCLUDE_HINT} ${LibGcrypt_INCLUDE_DIR}
)

find_library(LibGcrypt_LIBRARY
    NAMES ${LibGcrypt_NAMES} gcrypt libgcrypt
    HINTS ${PC_GCRYPT_LIBDIR} ${PC_GCRYPT_LIBRARY_DIRS} ${LIBGCRYPT_SCRIPT_LIB_HINT}
)

if (LibGcrypt_INCLUDE_DIR AND NOT LibGcrypt_VERSION)
    file(STRINGS ${LibGcrypt_INCLUDE_DIR}/gcrypt.h GCRYPT_H REGEX "^#define GCRYPT_VERSION ")
    string(REGEX REPLACE "^#define GCRYPT_VERSION \"([0-9.]\+)[^\"]*\".*$" "\\1" LibGcrypt_VERSION "${GCRYPT_H}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibGcrypt
    FOUND_VAR LibGcrypt_FOUND
    REQUIRED_VARS LibGcrypt_LIBRARY LibGcrypt_INCLUDE_DIR
                  LibGcrypt_GpgError_LIBRARY LibGcrypt_GpgError_INCLUDE_DIR
    VERSION_VAR LibGcrypt_VERSION
)

if (LibGcrypt_GpgError_LIBRARY AND NOT TARGET LibGcrypt::GpgError)
    add_library(LibGcrypt::GpgError UNKNOWN IMPORTED GLOBAL)
    set_target_properties(LibGcrypt::GpgError PROPERTIES
        IMPORTED_LOCATION "${LibGcrypt_GpgError_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LibGcrypt_GpgError_INCLUDE_DIR}"
    )
endif ()

if (LibGcrypt_LIBRARY AND NOT TARGET LibGcrypt::LibGcrypt)
    add_library(LibGcrypt::LibGcrypt UNKNOWN IMPORTED GLOBAL)
    set_target_properties(LibGcrypt::LibGcrypt PROPERTIES
        IMPORTED_LOCATION "${LibGcrypt_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${LibGcrypt_COMPILE_OPTIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${LibGcrypt_INCLUDE_DIR}"
    )
    target_link_libraries(LibGcrypt::LibGcrypt INTERFACE LibGcrypt::GpgError)
endif ()

mark_as_advanced(LibGcrypt_INCLUDE_DIR LibGcrypt_LIBRARY
    LibGcrypt_GpgError_INCLUDE_DIR LibGcrypt_GpgError_LIBRARY)

if (LibGcrypt_FOUND)
    set(LibGcrypt_LIBRARIES ${LibGcrypt_LIBRARY} ${LibGcrypt_GpgError_LIBRARY})
    set(LibGcrypt_INCLUDE_DIRS ${LibGcrypt_INCLUDE_DIR} ${LibGcrypt_GpgError_INCLUDE_DIR})
endif ()
