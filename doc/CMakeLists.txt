
add_subdirectory(manual-src)



########### install files ###############

install(FILES  xmlrpc/aria2mon  	xmlrpc/aria2rpc  	xmlrpc/README.txt TYPE DOC)
install(FILES  bash_completion/README.txt DESTINATION TYPE DOC)
install(FILES  bash_completion/aria2c DESTINATION TYPE DOC)



#original Makefile.am contents follow:

#SUBDIRS = manual-src
#
#doc_xmlrpcdir = $(docdir)/xmlrpc
#dist_doc_xmlrpc_DATA = xmlrpc/aria2mon \
#	xmlrpc/aria2rpc \
#	xmlrpc/README.txt
#
#doc_bashcompletiondir = $(docdir)/bash_completion
#dist_doc_bashcompletion_DATA = bash_completion/README.txt
#
#doc_bashcompletionscriptdir = @bashcompletiondir@
#dist_doc_bashcompletionscript_DATA = bash_completion/aria2c
