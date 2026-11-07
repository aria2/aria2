
include(GNUInstallDirs)

set(SPHINXOPTS "" CACHE STRING "Options for SPHINX")
set(SPHINXBUILD "sphinx-build" CACHE STRING "sphinx-build command")
set(PAPER "a4" CACHE STRING "Type of Paper")
set(BUILDDIR "_build" CACHE STRING "Build directory")

set(PAPEROPT_a4 "-D latex_elements.papersize=a4paper")
set(PAPEROPT_letter "-D latex_elements.papersize=letterpaper")

set(ALLSPHINXOPTS -d ${BUILDDIR}/doctrees ${PAPEROPT_${PAPER}} ${SPHINXOPTS} -c . ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_INSTALL_MANDIR ${CMAKE_INSTALL_PREFIX}/man)

set(MAN_OUTPUT_DIR "${BUILDDIR}/man")
set(MAN_PAGE "${MAN_OUTPUT_DIR}/aria2c.1")

if(HAVE_SPHINXBUILD)
    add_custom_command(
        OUTPUT ${MAN_PAGE}
        COMMAND ${CMAKE_COMMAND} -E echo "Running the make man command"
        COMMAND ${SPHINXBUILD} -b man ${ALLSPHINXOPTS} ${BUILDDIR}/man
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/aria2c.rst
        COMMAND ${CMAKE_COMMAND} -E echo "Building aria2 man page with Sphinx"
        VERBATIM
    )
else()
    add_custom_command(
        OUTPUT ${MAN_PAGE}
        COMMAND ${CMAKE_COMMAND} -E echo "WARNING: Building only stub man pages. Please install sphinx-build: pip install sphinx"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAN_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E touch ${MAN_PAGE}
        COMMENT "Creating stub man page"
        VERBATIM
    )
endif()

add_custom_target(customHelp
    COMMAND ${CMAKE_COMMAND} -E echo "Please use \\'make \\<target\\>\\' where \\<target\\> is one of:"
    COMMAND ${CMAKE_COMMAND} -E echo "  html       to make html standalone HTML files"
    COMMAND ${CMAKE_COMMAND} -E echo "  dirhtml    to make dirhtml HTML files named index.html in directories"
    COMMAND ${CMAKE_COMMAND} -E echo "  singlehtml to make singlehtml a single large HTML file"
    COMMAND ${CMAKE_COMMAND} -E echo "  pickle     to make pickle files"
    COMMAND ${CMAKE_COMMAND} -E echo "  json       to make JSON files"
    COMMAND ${CMAKE_COMMAND} -E echo "  htmlhelp   to make HTML files and a HTML help project"
    COMMAND ${CMAKE_COMMAND} -E echo "  qthelp     to make HTML files and a qthelp project"
    COMMAND ${CMAKE_COMMAND} -E echo "  devhelp    to make HTML files and a DevHelp project"
    COMMAND ${CMAKE_COMMAND} -E echo "  epub       to make an epub"
    COMMAND ${CMAKE_COMMAND} -E echo "  latex      to make LaTeX files, you can set PAPER=a4 or PAPER=letter"
    COMMAND ${CMAKE_COMMAND} -E echo "  latexpdf   to make LaTeX files and run them through pdflatex"
    COMMAND ${CMAKE_COMMAND} -E echo "  text       to make text files"
    COMMAND ${CMAKE_COMMAND} -E echo "  man        to make manual pages"
    COMMAND ${CMAKE_COMMAND} -E echo "  changes    to make an overview of all changed/added/deprecated items"
    COMMAND ${CMAKE_COMMAND} -E echo "  linkcheck  to check all external links for integrity"
    COMMAND ${CMAKE_COMMAND} -E echo "  doctest    to run all doctests embedded in the documentation \\(if enabled\\)"
    COMMAND ${CMAKE_COMMAND} -E echo ""
)

add_custom_target(docclean
    COMMAND
        rm libaria2api
        rm -rf ${BUILDDIR}/*
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}

    VERBATIM
)

add_custom_target(html
    COMMAND
        ${SPHINXBUILD} -b html ${ALLSPHINXOPTS} ${BUILDDIR}/html

    BYPRODUCTS ${BUILDDIR}/html
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished. The HTML pages are in ${BUILDDIR}/html"
    VERBATIM
)

add_custom_target(dirhtml
    COMMAND
        ${SPHINXBUILD} -b dirhtml ${ALLSPHINXOPTS} ${BUILDDIR}/dirhtml

    BYPRODUCTS ${BUILDDIR}/dirhtml
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished. The HTML pages are in ${BUILDDIR}/dirhtml"
    VERBATIM
)

add_custom_target(singlehtml
    COMMAND
        ${SPHINXBUILD} -b singlehtml ${ALLSPHINXOPTS} ${BUILDDIR}/singlehtml

    BYPRODUCTS ${BUILDDIR}/singlehtml
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished. The HTML pages are in ${BUILDDIR}/singlehtml"
    VERBATIM
)

# sudo apt-get install python3-sphinxcontrib.serializinghtml
add_custom_target(pickle
    COMMAND
        ${SPHINXBUILD} -b pickle ${ALLSPHINXOPTS} ${BUILDDIR}/pickle

    BYPRODUCTS ${BUILDDIR}/pickle
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished; now you can process the pickle files."
    VERBATIM
)

# sudo apt-get install python3-sphinxcontrib.serializinghtml (same as above)
add_custom_target(json
    COMMAND
        ${SPHINXBUILD} -b json ${ALLSPHINXOPTS} ${BUILDDIR}/json

    BYPRODUCTS ${BUILDDIR}/json
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished; now you can process the JSON files."
    VERBATIM
)

# sudo apt install python3-sphinxcontrib.htmlhelp
add_custom_target(htmlhelp
    COMMAND
        ${SPHINXBUILD} -b htmlhelp ${ALLSPHINXOPTS} ${BUILDDIR}/htmlhelp

    BYPRODUCTS ${BUILDDIR}/htmlhelp
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished; now you can run HTML Help Workshop with the .hhp project file in ${BUILDDIR}/htmlhelp."
    VERBATIM
)

# sudo apt-get install  python3-sphinxcontrib.qthelp
add_custom_target(qthelp
    COMMAND
        ${SPHINXBUILD} -b qthelp ${ALLSPHINXOPTS} ${BUILDDIR}/qthelp

    BYPRODUCTS ${BUILDDIR}/qthelp
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished; now you can run \"qcollectiongenerator\" with the"
                ".qhcp project file in ${BUILDDIR}/qthelp, like this:"
                "qcollectiongenerator ${BUILDDIR} /qthelp/aria2.qhcp"
    VERBATIM
)

# sudo apt install python3-sphinxcontrib.devhelp
add_custom_target(devhelp
    COMMAND
        ${SPHINXBUILD} -b devhelp ${ALLSPHINXOPTS} ${BUILDDIR}/devhelp

    BYPRODUCTS ${BUILDDIR}/devhelp
    ${CMAKE_COMMAND} -E echo "Build finished"
    ${CMAKE_COMMAND} -E echo "To view help file:"
    ${CMAKE_COMMAND} -E echo "  mkdir -p $$HOME/.local/devhelp/aria2"
    ${CMAKE_COMMAND} -E echo "  ls -s ${BUILDDIR}/devhelp $$HOME/.local/share/devhelp/aria2"
    ${CMAKE_COMMAND} -E echo "  devhelp"
    VERBATIM
)

add_custom_target(epub
    COMMAND
        ${SPHINXBUILD} -b epub ${ALLSPHINXOPTS} ${BUILDDIR}/epub

    BYPRODUCTS ${BUILDDIR}/epub
    ${CMAKE_COMMAND} -E echo "Build finished. The epub file is in ${BUILDDIR}/epub"
    VERBATIM
)

add_custom_target(latex
    COMMAND
        ${SPHINXBUILD} -b latex ${ALLSPHINXOPTS} ${BUILDDIR}/latex

    BYPRODUCTS ${BUILDDIR}/latex
    ${CMAKE_COMMAND} -E echo "Build finished. The LaTeX files are in ${BUILDDIR}/latex"
    ${CMAKE_COMMAND} -E echo "Run 'make' in that directory to run these through (pdf)latex (use 'make' latexpdf here to that automatically)"
    VERBATIM
)

add_custom_target(latexpdf
    COMMAND
        ${SPHINXBUILD} -b latex ${ALLSPHINXOPTS} ${BUILDDIR}/latex

    BYPRODUCTS ${BUILDDIR}/latex

    ${CMAKE_COMMAND} -E echo "Running LaTeX files through pdflatex..."
    ${CMAKE_MAKE_COMMAND} -C ${BUILDDIR}/latex all-pdf
    ${CMAKE_COMMAND} -E echo "pdflatex finished. the PDF files are in ${BUILDDIR}/latex"

    VERBATIM
)

add_custom_target(text
    COMMAND
        ${SPHINXBUILD} -b text ${ALLSPHINXOPTS} ${BUILDDIR}/text

    BYPRODUCTS ${BUILDDIR}/text
    ${CMAKE_COMMAND} -E echo "Build finished. The text files are in ${BUILDDIR}/text."
    VERBATIM
)

# man target, depends on the MAN_PAGE, which is output from a custom command.
# When 'make man' is executed the custom command is triggered
# To build it every time aria2c is built, add the ALL option
add_custom_target(man
    DEPENDS ${MAN_PAGE}
)

add_custom_target(changes
    COMMAND
        ${SPHINXBUILD} -b changes ${ALLSPHINXOPTS} ${BUILDDIR}/changes

    BYPRODUCTS ${BUILDDIR}/changes
    ${CMAKE_COMMAND} -E echo "The overview file is in ${BUILDDIR}/changes."
    VERBATIM
)

add_custom_target(linkcheck
    COMMAND
        ${SPHINXBUILD} -b linkcheck ${ALLSPHINXOPTS} ${BUILDDIR}/linkcheck

    BYPRODUCTS ${BUILDDIR}/linkcheck
    ${CMAKE_COMMAND} -E echo "Link check complete; look for any errors in the above output or in ${CMAKE_CURRENT_BINARY_DIR}/linkcheck/output.txt."
    VERBATIM
)

add_custom_target(doctest
    COMMAND
        ${SPHINXBUILD} -b doctest ${ALLSPHINXOPTS} ${BUILDDIR}/doctest

    BYPRODUCTS ${BUILDDIR}/doctest
    ${CMAKE_COMMAND} -E echo "Testing of doctests in the sources finished, look at the results in${CMAKE_CURRENT_BINARY_DIR}/doctest/output.txt"
    VERBATIM
)

install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${BUILDDIR}/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/en

    PATTERN "man" # man directory will be installed separately, see below
        EXCLUDE
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${BUILDDIR}/man/aria2c.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}
)