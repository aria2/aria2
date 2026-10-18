
set(SPHINXOPTS "" CACHE STRING "Options for SPHINX")
set(SPHINXBUILD "sphinx-build" CACHE STRING "sphinx-build command")
set(PAPER "" CACHE STRING "Type of Paper")
set(BUILDDIR "_build" CACHE STRING "Build directory")

set(PAPEROPT_a4 "-D latex_paper_size=a4")
set(PAPEROPT_letter "-D latext_paper_size=letter")
set(ALLSPHINXOPTS "-d ${CMAKE_CURRENT_BINARY_DIR}/doctrees ${PAPERPORT_${PAPER}} ${SPHINXOPTS} -c . ${CMAKE_CURRENT_SOURCE_DIR}")

set(man_MANS "${CMAKE_CURRENT_BINARY_DIR}/man/aria2c.1")

set(EXTRA_DIST "${man_MANS} aria2.rst libaria2.rst index.rst README.rst technical-notes.rst mkapiref.py")

if(HAVE_SPHINXBUILD)
    if(EXISTS aria2.rst)
        ${CMAKE_MAKE_PROGRAM} man
    endif()
else()

endif()  # TODOm here, is incomplete

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
        rm -rf ${CMAKE_CURRENT_BINARY_DIR}/*
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(html
    COMMAND
        ${SPHINXBUILD} -b html ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/html

    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/html
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished. The HTML pages are in ${CMAKE_CURRENT_BINARY_DIR}/html"
)

add_custom_target(dirhtml
    COMMAND
        ${SPHINXBUILD} -b dirhtml ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/dirhtml
    
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/dirhtml
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished. The HTML pages are in ${CMAKE_CURRENT_BINARY_DIR}/dirhtml"
)

add_custom_target(singlehtml
    COMMAND
        ${SPHINXBUILD} -b singlehtml ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/singlehtml
    
    BYRPODUCTS ${CMAKE_CURRENT_BINARY_DIR}/singlehtml
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished. The HTML pages are in ${CMAKE_CURRENT_BINARY_DIR}/singlehtml"
)

add_custom_target(pickle
    COMMAND
        ${SPHINXBUILD} -b pickle ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/pickle
    
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/pickle
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished; now you can process the pickle files."
)

add_custom_target(json
    COMMAND
        ${SPHINXBUILD} -b json ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/json
    
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/json
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished; now you can process the JSON files."
)

add_custom_target(htmlhelp
    COMMAND
        ${SPHINXBUILD} -b htmlhelp ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/htmlhelp
    
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/htmlhelp
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished; now you can run HTML Help Workshop with the .hhp project file in ${CMAKE_CURRENT_BINARY_DIR}/htmlhelp."
)

add_custom_target(qthelp
    COMMAND
        ${SPHINXBUILD} -b qthelp ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/qthelp
    
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/qthelp
    COMMAND ${CMAKE_COMMAND} -E echo "Build finished; now you can run \"qcollectiongenerator\" with the
             .qhcp project file in ${CMAKE_CURRENT_BINARY_DIR}/qthelp, like this:" \
            "# qcollectiongenerator ${CMAKE_CURRENT_BINARY_DIR} /qthelp/aria2.qhcp"
)

add_custom_target(devhelp
    COMMAND
        ${SPHINXBUILD} -b devhelp ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/devhelp
        ${CMAKE_COMMAND} -E echo "Build finished"
        ${CMAKE_COMMAND} -E echo "To view help file:"
        ${CMAKE_COMMAND} -E echo "# mkdir -p $$HOME/.local/devhelp/aria2"
        ${CMAKE_COMMAND} -E echo "# ls -s ${CMAKE_CURRENT_BINARY_DIR}/devhelp $$HOME/.local/share/devhelp/aria2"
        ${CMAKE_COMMAND} -E echo "# devhelp"
)

add_custom_target(epub
    COMMAND:
        ${SPHINXBUILD} -b epub ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/epub
        ${CMAKE_COMMAND} -E echo "Build finished. The epub file is in ${CMAKE_CURRENT_BINARY_DIR}/epub"
)

add_custom_target(latex
    COMMAND
        ${SPHINXBUILD} -b latex ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/latex
        ${CMAKE_COMMAND} -E echo "Build finished. The LaTeX files are in ${CMAKE_CURRENT_BINARY_DIR}/latex"
        ${CMAKE_COMMAND} -E echo "Run 'make' in that directory to run these through (pdf)latex" \
            "(use 'make' latexpdf here to that automatically)"
)

add_custom_target(latexpdf
    COMMAND
        ${SPHINXBUILD} -b latex ${ALLSPHINXOPTS} ${CAMKE_CURRENT_BINARY_DIR}/latex
        ${CMAKE_COMMAND} -E echo "Running LaTeX files through pdflatex..."
        ${MAKE_COMMAND} -C ${CMAKE_CURRENT_BINARY_DIR}/latex all-pdf
        ${CMAKE_COMMAND} -E echo "pdflatex finished. the PDF files are in ${CMAKE_CURRENT_BINARY_DIR}/latex"
)

add_custom_target(text
    COMMAND
        ${SPHINXBUILD} -b text ${ALLSPHINXOPTS} ${BUILDDIR}/text
        ${CMAKE_COMMAND} -E echo "Build finished. The text files are in $(BUILDDIR)/text."
)

add_custom_target(man
    COMMAND
        ${SPHINXBUILD} -b man ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/man
        ${CMAKE_COMMAND} -E echo "Build finished. The manual pages are in ${CMAKE_CURRENT_BINARY_DIR}/man."
)

add_custom_target(linkcheck
    COMMAND
        ${SPHINXBUILD} -b linkcheck ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/linkcheck
        ${CMAKE_COMMAND} -E echo "Link check complete; look for any errors in the above output or in ${CMAKE_CURRENT_BINARY_DIR}/linkcheck/output.txt."
)

add_custom_target(doctest
    COMMAND
        ${SPHINXBUILD} -b doctest ${ALLSPHINXOPTS} ${CMAKE_CURRENT_BINARY_DIR}/doctest
        ${CMAKE_COMMAND} -E echo "Testing of doctests in the sources finished, look at the results in${CMAKE_CURRENT_BINARY_DIR}/doctest/output.txt"
)