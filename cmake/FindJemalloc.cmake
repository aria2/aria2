FIND_PATH( Jemalloc_INCLUDE_DIR
  NAMES
    jemalloc/jemalloc.h
  PATHS
    /usr/include
    /usr/include/jemalloc
    /usr/local/include
    /usr/local/include/jemalloc
    $ENV{JEMALLOC_ROOT}
    $ENV{JEMALLOC_ROOT}/include
    ${CMAKE_SOURCE_DIR}/externals/jemalloc
)
FIND_LIBRARY(Jemalloc_LIBRARY 
  NAMES
    jemalloc libjemalloc JEMALLOC
  PATHS
    /usr/lib
    /usr/lib/jemalloc
    /usr/local/lib
    /usr/local/lib/jemalloc
    /usr/local/jemalloc/lib
    $ENV{JEMALLOC_ROOT}/lib
    $ENV{JEMALLOC_ROOT}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  Jemalloc
  REQUIRED_VARS
  Jemalloc_LIBRARY
  Jemalloc_INCLUDE_DIR
)

mark_as_advanced( Jemalloc_LIBRARY Jemalloc_INCLUDE_DIR )
