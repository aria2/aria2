# Standard FIND_PACKAGE module for libuv, sets the following variables:
#   - LIBUV_FOUND
#   - LIBUV_INCLUDE_DIRS (only if LIBUV_FOUND)
#   - LIBUV_LIBRARIES (only if LIBUV_FOUND)

# Try to find the header
FIND_PATH(LIBUV_INCLUDE_DIR NAMES uv.h)

# Try to find the library
FIND_LIBRARY(LIBUV_LIBRARY NAMES uv libuv)

# Parse out version header
if(LIBUV_INCLUDE_DIR)
  set(_version_major_regex "^#define[ \t]+UV_VERSION_MAJOR[ \t]+([0-9]+).*$")
  set(_version_minor_regex "^#define[ \t]+UV_VERSION_MINOR[ \t]+([0-9]+).*$")
  set(_version_patch_regex "^#define[ \t]+UV_VERSION_PATCH[ \t]+([0-9]+).*$") 
  file(STRINGS "${LIBUV_INCLUDE_DIR}/uv/version.h" LIBUV_VERSION_MAJOR REGEX ${_version_major_regex})
  file(STRINGS "${LIBUV_INCLUDE_DIR}/uv/version.h" LIBUV_VERSION_MINOR REGEX ${_version_minor_regex})
  file(STRINGS "${LIBUV_INCLUDE_DIR}/uv/version.h" LIBUV_VERSION_PATCH REGEX ${_version_patch_regex})
  string(REGEX REPLACE ${_version_major_regex} "\\1" LIBUV_VERSION_MAJOR ${LIBUV_VERSION_MAJOR})
  string(REGEX REPLACE ${_version_minor_regex} "\\1" LIBUV_VERSION_MINOR ${LIBUV_VERSION_MINOR})
  string(REGEX REPLACE ${_version_patch_regex} "\\1" LIBUV_VERSION_PATCH ${LIBUV_VERSION_PATCH})
  set(LIBUV_VERSION "${LIBUV_VERSION_MAJOR}.${LIBUV_VERSION_MINOR}.${LIBUV_VERSION_PATCH}")
  unset(_version_major_regex)
  unset(_version_minor_regex)
  unset(_version_patch_regex)
endif()

# Handle the QUIETLY/REQUIRED arguments, set LIBUV_FOUND if all variables are
# found
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  LIBUV
  REQUIRED_VARS
  LIBUV_LIBRARY
  LIBUV_INCLUDE_DIR
  VERSION_VAR LIBUV_VERSION
  NAME_MISMATCHED
)



# Hide internal variables
MARK_AS_ADVANCED(LIBUV_INCLUDE_DIR LIBUV_LIBRARY)

# Set standard variables
IF(LIBUV_FOUND)
    SET(LIBUV_INCLUDE_DIRS "${LIBUV_INCLUDE_DIR}")
    SET(LIBUV_LIBRARIES "${LIBUV_LIBRARY}")
    add_library(LIBUV::LIBUV UNKNOWN IMPORTED GLOBAL)
    set_target_properties(LIBUV::LIBUV PROPERTIES
    IMPORTED_LOCATION "${LIBUV_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${LIBUV_COMPILE_OPTIONS}"
    INTERFACE_INCLUDE_DIRECTORIES "${LIBUV_INCLUDE_DIRS}"
  )
ENDIF()
