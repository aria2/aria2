# - Find Nettle
# Find the Nettle include directory and library
#
#  NETTLE_INCLUDE_DIR    - where to find <nettle/sha.h>, etc.
#  NETTLE_LIBRARIES      - List of libraries when using libnettle.
#  NETTLE_FOUND          - True if libnettle found.

IF (NETTLE_INCLUDE_DIR)
  # Already in cache, be silent
  SET(NETTLE_FIND_QUIETLY TRUE)
ENDIF (NETTLE_INCLUDE_DIR)

FIND_PATH(NETTLE_INCLUDE_DIR nettle/md5.h nettle/ripemd160.h nettle/sha.h)
FIND_LIBRARY(NETTLE_LIBRARY NAMES nettle libnettle)

if(NETTLE_INCLUDE_DIR)
  set(_version_regex_major "^#define[ \t]+NETTLE_VERSION_MAJOR[ \t]+([0-9])$")
  set(_version_regex_minor "^#define[ \t]+NETTLE_VERSION_MINOR[ \t]+([0-9])$")
  file(STRINGS "${NETTLE_INCLUDE_DIR}/nettle/version.h" NETTLE_VERSION_MAJOR REGEX ${_version_regex_major})
  file(STRINGS "${NETTLE_INCLUDE_DIR}/nettle/version.h" NETTLE_VERSION_MINOR REGEX ${_version_regex_minor})
  string(REGEX REPLACE "${_version_regex_major}" "\\1" NETTLE_VERSION_MAJOR ${NETTLE_VERSION_MAJOR})
  string(REGEX REPLACE "${_version_regex_minor}" "\\1" NETTLE_VERSION_MINOR ${NETTLE_VERSION_MINOR})
  set(NETTLE_VERSION "${NETTLE_VERSION_MAJOR}.${NETTLE_VERSION_MINOR}")
  unset(_version_regex_major)
  unset(_version_regex_minor)
endif()

# handle the QUIETLY and REQUIRED arguments and set NETTLE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  NETTLE
  REQUIRED_VARS NETTLE_LIBRARY NETTLE_INCLUDE_DIR
  VERSION_VAR NETTLE_VERSION
  NAME_MISMATCHED
)

IF(NETTLE_FOUND)
  SET(NETTLE_LIBRARIES ${NETTLE_LIBRARY})
  
  add_library(Nettle::Nettle UNKNOWN IMPORTED GLOBAL)
  set_target_properties(Nettle::Nettle PROPERTIES
    IMPORTED_LOCATION "${NETTLE_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${NETTLE_COMPILE_OPTIONS}"
    INTERFACE_INCLUDE_DIRECTORIES "${NETTLE_INCLUDE_DIR}"
  )
ENDIF(NETTLE_FOUND)
mark_as_advanced(NETTLE_INCLUDE_DIR NETTLE_LIBRARY NETTLE_LIBRARIES NETTLE_VERSION NETTLE_VERSION_MINOR NETTLE_VERSION_MAJOR)


