find_package(PackageHandleStandardArgs)

find_program( RST2HTML_EXECUTABLE NAMES rst2html.py rst2html DOC "Python Docutils reStructuredText to HTML converter")

if (RST2HTML_EXECUTABLE)
  if (WIN32)
    find_package(PythonInterp)
    if (NOT PYTHON_EXECUTABLE)
      message(FATAL_ERROR "Can't find Python interpreter, required by Docutils")
    endif()
    execute_process(COMMAND ${PYTHON_EXECUTABLE} ${RST2HTML_EXECUTABLE} --version OUTPUT_VARIABLE DOCUTILS_VERSION_STR)
  else()
    execute_process(COMMAND ${RST2HTML_EXECUTABLE} --version OUTPUT_VARIABLE DOCUTILS_VERSION_STR)
  endif()

  if (NOT "${DOCUTILS_VERSION_STR}" STREQUAL "")
    string(REGEX MATCHALL "([^\ ]+\ |[^\ ]+$)" DOCUTILS_VERSION_PARTS "${DOCUTILS_VERSION_STR}")
    list(GET DOCUTILS_VERSION_PARTS 2 DOCUTILS_VERSION_STR)
    string(REGEX REPLACE "[ \t]+$" "" DOCUTILS_VERSION_STR ${DOCUTILS_VERSION_STR})
    string(REGEX REPLACE "^[ \t]+" "" DOCUTILS_VERSION_STR ${DOCUTILS_VERSION_STR})
    string(REGEX REPLACE ",$" "" DOCUTILS_VERSION_STR ${DOCUTILS_VERSION_STR})

    set(DOCUTILS_VERSION ${DOCUTILS_VERSION_STR})
  endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Docutils
  REQUIRED_VARS RST2HTML_EXECUTABLE DOCUTILS_VERSION
  VERSION_VAR DOCUTILS_VERSION
)

mark_as_advanced(RST2HTML_EXECUTABLE)
