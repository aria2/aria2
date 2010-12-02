man_MANS = aria2c.1
EXTRA_DIST = $(man_MANS)
dist_doc_DATA = aria2c.1.html aria2c.1.asciidoc
doc_xmlrpcdir = $(docdir)/xmlrpc
dist_doc_xmlrpc_DATA = xmlrpc/aria2mon \
	xmlrpc/aria2rpc \
	xmlrpc/README.txt

if HAVE_A2X
aria2c.1: aria2c.1.asciidoc
	@A2X@ -f manpage aria2c.1.asciidoc
endif # HAVE_A2X

if HAVE_ASCIIDOC
aria2c.1.html: aria2c.1.asciidoc
	@ASCIIDOC@ -d manpage -b xhtml11 -a toc -a max-width=55em aria2c.1.asciidoc
endif # HAVE_ASCIIDOC
