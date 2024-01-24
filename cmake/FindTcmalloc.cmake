# - Find Tcmalloc
# Find the native Tcmalloc includes and library
#
#  Tcmalloc_INCLUDE_DIR - where to find Tcmalloc.h, etc.
#  Tcmalloc_LIBRARIES   - List of libraries when using Tcmalloc.
#  Tcmalloc_FOUND       - True if Tcmalloc found.

find_path(Tcmalloc_INCLUDE_DIR gperftools/tcmalloc.h NO_DEFAULT_PATH PATHS
  ${HT_DEPENDENCY_INCLUDE_DIR}
  /usr/include
  /opt/local/include
  /usr/local/include
)

if (USE_TCMALLOC)
  set(Tcmalloc_NAMES libtcmalloc tcmalloc)
else ()
  set(Tcmalloc_NAMES libtcmalloc libtcmalloc_minimal tcmalloc_minimal tcmalloc)
endif ()

find_library(Tcmalloc_LIBRARY NO_DEFAULT_PATH
  NAMES ${Tcmalloc_NAMES}
  PATHS ${HT_DEPENDENCY_LIB_DIR} /lib /usr/lib /usr/local/lib /opt/local/lib
)

if (Tcmalloc_INCLUDE_DIR AND Tcmalloc_LIBRARY)
  set( Tcmalloc_LIBRARIES ${Tcmalloc_LIBRARY} )
endif ()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  Tcmalloc
  REQUIRED_VARS
  Tcmalloc_LIBRARY
  Tcmalloc_INCLUDE_DIR
)


mark_as_advanced(
  Tcmalloc_LIBRARY
  Tcmalloc_INCLUDE_DIR
  )
