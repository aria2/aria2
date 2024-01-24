# Try to find the GNU Multiple Precision Arithmetic Library (GMP)
# See http://gmplib.org/

if (GMP_INCLUDES AND GMP_LIBRARIES)
  set(GMP_FIND_QUIETLY TRUE)
endif (GMP_INCLUDES AND GMP_LIBRARIES)

find_path(GMP_INCLUDE_DIR
  NAMES
  gmp.h
  PATHS
  $ENV{GMPDIR}
  ${INCLUDE_INSTALL_DIR}
)

find_library(GMP_LIBRARY gmp PATHS $ENV{GMPDIR} ${LIB_INSTALL_DIR})

if(GMP_LIBRARY)
  set(GMP_LIBRARIES ${GMP_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GMP 
  REQUIRED_VARS GMP_INCLUDE_DIR GMP_LIBRARIES
)

if(GMP_FOUND)
  add_library(GMP::GMP UNKNOWN IMPORTED GLOBAL)
  set_target_properties(GMP::GMP PROPERTIES
    IMPORTED_LOCATION "${GMP_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${GMP_COMPILE_OPTIONS}"
    INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}"
  )

endif()
mark_as_advanced(GMP_INCLUDES GMP_LIBRARIES)

