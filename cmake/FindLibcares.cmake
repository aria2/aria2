# - Try to find libcares
# Once done this will define
#  LIBCARES_FOUND        - System has libcares
#  LIBCARES_INCLUDE_DIRS - The libcares include directories
#  LIBCARES_LIBRARIES    - The libraries needed to use libcares

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBCARES QUIET libcares)

find_path(LIBCARES_INCLUDE_DIR
  NAMES ares.h
  HINTS ${PC_LIBCARES_INCLUDE_DIRS}
)
find_library(LIBCARES_LIBRARY
  NAMES cares
  HINTS ${PC_LIBCARES_LIBRARY_DIRS}
)

if(LIBCARES_INCLUDE_DIR)
  set(_version_regex "^#define[ \t]+ARES_VERSION_STR[ \t]+\"([^\"]+)\".*")
  file(STRINGS "${LIBCARES_INCLUDE_DIR}/ares_version.h"
    LIBCARES_VERSION REGEX "${_version_regex}")
  string(REGEX REPLACE "${_version_regex}" "\\1"
    LIBCARES_VERSION "${LIBCARES_VERSION}")
  unset(_version_regex)
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBCARES_FOUND to TRUE
# if all listed variables are TRUE and the requested version matches.
find_package_handle_standard_args(Libcares REQUIRED_VARS
                                  LIBCARES_LIBRARY LIBCARES_INCLUDE_DIR
                                  VERSION_VAR LIBCARES_VERSION)

if(LIBCARES_FOUND)
  set(LIBCARES_LIBRARIES     ${LIBCARES_LIBRARY})
  set(LIBCARES_INCLUDE_DIRS  ${LIBCARES_INCLUDE_DIR})

  add_library(Libcares::Libcares UNKNOWN IMPORTED GLOBAL)
  set_target_properties(Libcares::Libcares PROPERTIES
    IMPORTED_LOCATION "${LIBCARES_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${LIBCARES_COMPILE_OPTIONS}"
    INTERFACE_INCLUDE_DIRECTORIES "${LIBCARES_INCLUDE_DIR}"
  )

endif()


mark_as_advanced(LIBCARES_INCLUDE_DIR LIBCARES_LIBRARY)
