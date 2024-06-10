include(FindPackageHandleStandardArgs)

# We are likely to find Sphinx near the Python interpreter
find_package(PythonInterp)
if(PYTHONINTERP_FOUND)
    get_filename_component(_PYTHON_DIR "${PYTHON_EXECUTABLE}" DIRECTORY)
    set(
        _PYTHON_PATHS
        "${_PYTHON_DIR}"
        "${_PYTHON_DIR}/bin"
        "${_PYTHON_DIR}/Scripts")
endif()

find_program(
    SPHINX_EXECUTABLE
    NAMES sphinx-build sphinx-build.exe
    HINTS ${_PYTHON_PATHS})
mark_as_advanced(SPHINX_EXECUTABLE)

if(SPHINX_EXECUTABLE)

    if(WIN32 AND PYTHON_EXECUTABLE)
        execute_process(COMMAND ${PYTHON_EXECUTABLE} ${SPHINX_EXECUTABLE} --version OUTPUT_VARIABLE SPHINX_VERSION_STR)
    else()
        execute_process(COMMAND ${SPHINX_EXECUTABLE} --version OUTPUT_VARIABLE SPHINX_VERSION_STR)
    endif()

    if (NOT "${SPHINX_VERSION_STR}" STREQUAL "")
        if (SPHINX_VERSION_STR MATCHES "sphinx-build ([0-9]+\\.[0-9]+(\\.|a?|b?)([0-9]*)(b?)([0-9]*))")
            set (SPHINX_VERSION "${CMAKE_MATCH_1}")
        elseif (_Sphinx_VERSION MATCHES "Sphinx v([0-9]+\\.[0-9]+(\\.|b?)([0-9]*)(b?)([0-9]*))")
            set (SPHINX_VERSION "${CMAKE_MATCH_1}")
        elseif (_Sphinx_VERSION MATCHES "Sphinx \\(sphinx-build\\) ([0-9]+\\.[0-9]+(\\.|a?|b?)([0-9]*)(b?)([0-9]*))")
            set (SPHINX_VERSION "${CMAKE_MATCH_1}")
        endif ()
    endif()
endif()

find_package_handle_standard_args(
    Sphinx 
    REQUIRED_VARS SPHINX_EXECUTABLE SPHINX_VERSION
    VERSION_VAR SPHINX_VERSION
)
