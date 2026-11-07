
set(doc_xmlrpcdir "${CMAKE_INSTALL_DOCDIR}/xmlrpc")
set(dis_doc_xmlrpc_DATA "xmlrpc/aria2mon xmlrpc/aria2rpc xmlrpc/README.txt")
set(doc_bashcompletiondir "${CMAKE_INSTALL_DOCDIR}/bash_completion")

set(dist_doc_bashcompletion_DATA "bash_completion/README.txt")

set(doc_bashcompletionscriptdir ${bashcompletiondir})  #${bashcompeltiondir} comes from the main CMakeLists.txt
set(dist_doc_bashcompletionscript_DATA "bash_completion/aria2c")

add_subdirectory(manual-src)
